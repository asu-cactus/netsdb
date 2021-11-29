import pandas as pd
import blocker
import gc
from lsh import my_timer, minHash, minHashLSH
from tqdm import tqdm


def mistique_exact_match(model, model_storage, x_test, y_test, indexer, 
                        stop_acc_drop=0.04, eval_step=5):
    """Perform deduplication on a keras model with MISTIQUE Exact Match.

    Args:
        model (tf.keras.Models): block to be deduplicated
        model_storage (model storage): model storage of the given model
        x_test (numpy array): evaluation dataset features matrix
        y_label (numpy array): evaluation dataset labels
        indexer (indexer.Indexer): object to store unique blocks
        stop_acc_drop (float): hard stop to deduplication when accuracy reduction is 
                               larger than the value, ranges from 0 to 1, default=0.04 
        eval_step (int): evaluate the model on evaluation dataset after k steps, defaule=5
        

    """

    block_size_x = indexer.block_size_x
    block_size_y = indexer.block_size_y
    block_magnitudes = model_storage['block_magnitude']

    # Init results storage
    list_b1_index = []
    list_b2_index = []
    list_b2_sim = []
    list_acc = []
    list_is_deduplicate = []
    list_search_time = []
    list_total_time = []
    list_eval_time = []
    
    # Obtain original accuracy
    _, ori_acc = model.evaluate(x_test, y_test)

    step = 1

    # Init some timers
    timer_total = my_timer.Timer()
    timer_search = my_timer.Timer()
    timer_eval = my_timer.Timer()

    timer_exact_match_index_build = my_timer.Timer()
    timer_exact_match_index_build.tic()
    hash_table = {}
    # Build exact hash index
    for i in tqdm(range(indexer.num_total)):
        b2 = indexer.list_blocks[i]
        b2_hash = hash(b2.tobytes())
        hash_table[b2_hash] = i
    t_build_lsh_index = timer_exact_match_index_build.toc()

    print('Exact hash index build time: {} ms'.format(t_build_lsh_index))


    # Get the index of the block gets deduplicated first
    for magnitude_k, magnitude_v in tqdm(sorted(block_magnitudes.items(), key=lambda item: item[1]), total=len(block_magnitudes)):
        timer_total.tic()

        weight_idx, block_x_idx, block_y_idx = magnitude_k
        b1_index = (weight_idx, block_x_idx, block_y_idx)
        
        # Flag to indicate whether the block can be deduplicated
        has_dedup = False  
        # Most similar block
        max_b2 = ''  
        # Maximum similarity
        max_sim = 0  
        # Index of the most similar block
        max_b2_index = ''  
        # Time
        t_search_time = ''
        t_eval_time = ''
        t_total_time = ''
        
        # Here is a trick: do not deduplicate the layer that has only 1 block will
        # gain better compression ratio and less accuracy reduction.
        if model_storage['weights_block_num'][weight_idx][0]*model_storage['weights_block_num'][weight_idx][1] > 1:
            # The block needs to be deduplicate
            b1 = model_storage['weights_padded'][weight_idx][block_x_idx*block_size_x: (
                block_x_idx+1)*block_size_x, block_y_idx*block_size_y: (block_y_idx+1)*block_size_y]

            # Tic for most similar block search
            timer_search.tic()
            query_result = []

            b1_hash = hash(b1.tobytes())
            if b1_hash in hash_table.keys():
                query_result = [hash_table[b1_hash]]

            for b2_index in query_result:
                if b1_index == b2_index:
                    continue
                b2 = indexer.list_blocks[b2_index]
                if (b1 == b2).all():
                    max_sim = 1
                    max_b2 = b2
                    max_b2_index = b2_index
                    has_dedup = True
            
            t_search_time = timer_search.toc()

        # If there is a deduplicable block, then deduplicate it
        if has_dedup:
            model_storage['weights_padded'][weight_idx][block_x_idx*block_size_x: (
                block_x_idx+1)*block_size_x, block_y_idx*block_size_y: (block_y_idx+1)*block_size_y] = max_b2
            
            weights_restored = blocker.reconstruct_weight_2d(model_storage)
            model.set_weights(weights_restored)

            del weights_restored
            gc.collect()
            step += 1
            
        # Evaluation at each eval_step
        if step % eval_step == 0:
            timer_eval.tic()
            _, acc = model.evaluate(x_test, y_test)
            t_eval_time = timer_eval.toc()
        else:
            if len(list_acc) != 0:
                acc = list_acc[-1]
            else:
                acc = ori_acc
        
        t_total_time = timer_total.toc()

        list_b1_index.append((weight_idx, block_x_idx, block_y_idx))
        list_b2_index.append(max_b2_index)
        list_b2_sim.append(max_sim)
        list_acc.append(acc)
        list_is_deduplicate.append(has_dedup)
        list_search_time.append(t_search_time)
        list_total_time.append(t_total_time)
        list_eval_time.append(t_eval_time)

        print('Progress: ', len(list_b2_index), '/',
                len(block_magnitudes), ' Max sim', max_sim, 'Duplicate block', magnitude_k)
    
        # Stop the deduplication once the accuracy reduction is larger than a threshold
        if ori_acc - acc > stop_acc_drop:
            break

    # Add the remaining not-deduplicated blocks to the result sheet
    for weight_idx in range(len(model_storage['weights_padded'])):
            m_weight_block_x_num, m_weight_block_y_num = model_storage[
                'weights_block_num'][weight_idx]
            for block_x_idx in range(m_weight_block_x_num):
                for block_y_idx in range(m_weight_block_y_num):
                    
                    b_index = (weight_idx, block_x_idx, block_y_idx)
                    if b_index in list_b1_index:
                        continue

                    list_b1_index.append(b_index)
                    list_b2_index.append('')
                    list_b2_sim.append('')
                    list_acc.append('')
                    list_is_deduplicate.append(False)
                    list_search_time.append('')
                    list_total_time.append('')
                    list_eval_time.append('')

    dedup_df = pd.DataFrame({'duplicate_block_idx': list_b1_index, 'deduplicate_block_idx': list_b2_index,
                                'block_similarity': list_b2_sim, 'model_accuracy': list_acc,
                                'is_deduplicated': list_is_deduplicate,
                                'search_time': list_search_time,
                                'eval_time': list_eval_time,
                                'total_time': list_total_time})
    dedup_df.head()

    return dedup_df

def actual_jaccard(data1, data2):
    """A util function to get the real 'jaccard similarity'

    Args:
        data1: numpy array
        data2: numpy array
    
    Returns:
        float: the real jaccard similarity
    """

    s1 = set(data1)
    s2 = set(data2)
    actual_jaccard = float(len(s1.intersection(s2)) / len(s1.union(s2)))

    return actual_jaccard


def mistique_approximate_match(model, model_storage, x_test, y_test, indexer, 
                        stop_acc_drop=0.04, eval_step=5):
    """Perform a deduplication on a keras model with MISTIQUE Approximate Match.

    Args:
        model (tf.keras.Models): block to be deduplicated
        model_storage (model storage): model storage of the given model
        x_test (numpy array): evaluation dataset features matrix
        y_label (numpy array): evaluation dataset labels
        indexer (indexer.Indexer): object to store unique blocks
        stop_acc_drop (float): hard stop to deduplication when accuracy reduction is 
                               larger than the value, ranges from 0 to 1, default=0.04 
        eval_step (int): evaluate the model on evaluation dataset after k steps, defaule=5
        

    """

    block_size_x = indexer.block_size_x
    block_size_y = indexer.block_size_y
    block_magnitudes = model_storage['block_magnitude']

    # Init results storage
    list_b1_index = []
    list_b2_index = []
    list_b2_sim = []
    list_acc = []
    list_is_deduplicate = []
    list_search_time = []
    list_total_time = []
    list_eval_time = []
    
    # Obtain original accuracy
    _, ori_acc = model.evaluate(x_test, y_test)

    step = 1

    # Init some timers
    timer_total = my_timer.Timer()
    timer_search = my_timer.Timer()
    timer_eval = my_timer.Timer()

    timer_exact_match_index_build = my_timer.Timer()
    timer_exact_match_index_build.tic()
    # Build minhash index
    minhashlsh = minHashLSH.MinHashLSH(threshold=0.2, num_permu=48)
    for i in tqdm(range(indexer.num_total)):
        b2 = indexer.list_blocks_discritized[i]
        b2_minhash = minHash.MinHash(num_permu=48)
        b2_minhash.add_a_matrix(b2)
        minhashlsh.insert(i, b2_minhash)

    t_build_lsh_index = timer_exact_match_index_build.toc()

    print('Exact minhash index build time: {} ms'.format(t_build_lsh_index))


    # Get the index of the block gets deduplicated first
    for magnitude_k, magnitude_v in tqdm(sorted(block_magnitudes.items(), key=lambda item: item[1]), total=len(block_magnitudes)):
        timer_total.tic()

        weight_idx, block_x_idx, block_y_idx = magnitude_k
        b1_index = (weight_idx, block_x_idx, block_y_idx)
        
        # Flag to indicate whether the block can be deduplicated
        has_dedup = False  
        # Most similar block
        max_b2 = ''  
        # Maximum similarity
        max_sim = 0  
        # Index of the most similar block
        max_b2_index = ''  
        # Time
        t_search_time = ''
        t_eval_time = ''
        t_total_time = ''
        
        # Here is a trick: do not deduplicate the layer that has only 1 block will
        # gain better compression ratio and less accuracy reduction.
        if model_storage['weights_block_num'][weight_idx][0]*model_storage['weights_block_num'][weight_idx][1] > 1:
            # The block needs to be deduplicated
            b1 = model_storage['weights_padded'][weight_idx][block_x_idx*block_size_x: (
                block_x_idx+1)*block_size_x, block_y_idx*block_size_y: (block_y_idx+1)*block_size_y]
            
            b1_discritized = model_storage['weights_padded_discritized'][weight_idx][block_x_idx*block_size_x: (
                block_x_idx+1)*block_size_x, block_y_idx*block_size_y: (block_y_idx+1)*block_size_y]

            # Tic for most similar block search
            timer_search.tic()
            minhash = minHash.MinHash(num_permu=48)
            minhash.add_a_matrix(b1_discritized)
            query_result = minhashlsh.query(minhash)
            print('len candidate', len(query_result))
            for b2_index in query_result:
                b2 = indexer.list_blocks[b2_index]
                b2_discritized = indexer.list_blocks_discritized[b2_index]
                block_sim = actual_jaccard(b1_discritized.reshape(-1), b2_discritized.reshape(-1))
                if block_sim > max_sim:
                    max_sim = block_sim
                    max_b2 = b2
                    max_b2_index = b2_index
                    has_dedup = True
            
            t_search_time = timer_search.toc()

        # If there is a deduplicable block, then deduplicate it
        if has_dedup:
            model_storage['weights_padded'][weight_idx][block_x_idx*block_size_x: (
                block_x_idx+1)*block_size_x, block_y_idx*block_size_y: (block_y_idx+1)*block_size_y] = max_b2
            
            weights_restored = blocker.reconstruct_weight_2d(model_storage)
            model.set_weights(weights_restored)

            del weights_restored
            gc.collect()
            step += 1
            
        # Evaluation at each eval_step
        if step % eval_step == 0:
            timer_eval.tic()
            _, acc = model.evaluate(x_test, y_test)
            t_eval_time = timer_eval.toc()
        else:
            if len(list_acc) != 0:
                acc = list_acc[-1]
            else:
                acc = ori_acc
        
        t_total_time = timer_total.toc()

        list_b1_index.append((weight_idx, block_x_idx, block_y_idx))
        list_b2_index.append(max_b2_index)
        list_b2_sim.append(max_sim)
        list_acc.append(acc)
        list_is_deduplicate.append(has_dedup)
        list_search_time.append(t_search_time)
        list_total_time.append(t_total_time)
        list_eval_time.append(t_eval_time)

        print('Progress: ', len(list_b2_index), '/',
                len(block_magnitudes), ' Max sim', max_sim, 'Duplicate block', magnitude_k)
    
        # Stop the deduplication once the accuracy reduction is larger than a threshold
        if ori_acc - acc > stop_acc_drop:
            break

    # Add the remaining not-deduplicated blocks to the result sheet
    for weight_idx in range(len(model_storage['weights_padded'])):
            m_weight_block_x_num, m_weight_block_y_num = model_storage[
                'weights_block_num'][weight_idx]
            for block_x_idx in range(m_weight_block_x_num):
                for block_y_idx in range(m_weight_block_y_num):
                    
                    b_index = (weight_idx, block_x_idx, block_y_idx)
                    if b_index in list_b1_index:
                        continue

                    list_b1_index.append(b_index)
                    list_b2_index.append('')
                    list_b2_sim.append('')
                    list_acc.append('')
                    list_is_deduplicate.append(False)
                    list_search_time.append('')
                    list_total_time.append('')
                    list_eval_time.append('')

    dedup_df = pd.DataFrame({'duplicate_block_idx': list_b1_index, 'deduplicate_block_idx': list_b2_index,
                                'block_similarity': list_b2_sim, 'model_accuracy': list_acc,
                                'is_deduplicated': list_is_deduplicate,
                                'search_time': list_search_time,
                                'eval_time': list_eval_time,
                                'total_time': list_total_time})
    dedup_df.head()

    return dedup_df