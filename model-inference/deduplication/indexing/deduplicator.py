import gc
import blocker
import numpy as np
import pandas as pd
import tensorflow as tf
import blocker
import tensorflow_hub as hub
from lsh import my_timer, l2lsh
from tqdm import tqdm

def finetune_model(model, model_storage, list_dedup_blocks_idx, list_dedup_weights, x_train, y_train, block_size_x, block_size_y, epochs=1, batch_size=128):
    """Fine-tune a model
    Args:
        model (tf.keras.Models): model to get fine-tuned
        model_storage (model storage): model storage created for the given mdoel
        list_dedup_blocks_idx (list): a list to store the indexes of the blocks that 
                                    have been deduplicated
        list_dedup_weightrs (list): a list to store the blocks that used for deduplication
        x_train (numpy array): fine-tune dataset features matrix
        y_train (numpy array): fine-tune dataset labels  
        block_size_x (int): width of a block
        block_size_y (int): height of a block
        epochs (int): number of epochs for each fine-tune, default=1
        batch_size (int): batch size of fine-tune, default=128
    
    Return:
        tf.keras.Models: fine-tuned model
        model_storage: an updated model storage 

    """
    for _ in tqdm(range(epochs), leave=False):  
        sample_index = np.random.choice(len(x_train), batch_size)
        x_train_sample = x_train[sample_index]
        y_train_sample = y_train[sample_index]
        model.fit(x_train_sample, y_train_sample, epochs=1,
                  batch_size=batch_size, verbose=1)

        # Somehow if the model is too large the fine-tune process may be very slow
        # a trick to solve it is saving to the disk then loading it back
        model.save('temp.h5')
        del model
        del model_storage
        gc.collect()
        model = tf.keras.models.load_model('temp.h5', custom_objects={'KerasLayer':hub.KerasLayer})
        model_storage = blocker.block_model_2d(model, block_size_x, block_size_y)

        # Reset the weights of previous deduplicated blocks
        for w_id in range(len(list_dedup_blocks_idx)):
            weight_idx, block_x_idx, block_y_idx = list_dedup_blocks_idx[w_id]
            model_storage['weights_padded'][weight_idx][block_x_idx*block_size_x: (
                block_x_idx+1)*block_size_x, block_y_idx*block_size_y: (block_y_idx+1)*block_size_y] = list_dedup_weights[w_id]

        # Set weights back to model
        weights_restored = blocker.reconstruct_weight_2d(model_storage)
        model.set_weights(weights_restored)

    return model, model_storage


def deduplicate_model(model, model_storage,
                      x_test=None, y_test=None, indexer=None, fp=0.01, sim=0.7, stop_acc_drop=0.04, eval_step=5, 
                      eval_batch_size=512, finetune=False, x_train=None, y_train=None, ft_epochs=1, ft_batch_size=128,
                      ft_step=5, use_lsh=False, ds_test=None, ds_test_steps=None, is_updated_model=False,
                      path_to_ori_model=None, path_to_ori_output=None):
    """Perform deduplication on a keras model. If fine-tune is enabled, x_train and y_train are needed.

    Args:
        model (tf.keras.Models): block to be deduplicated
        model_storage (model storage): model storage of the given model
        x_test (numpy array): evaluation dataset features matrix
        y_label (numpy array): evaluation dataset labels
        indexer (indexer.Indexer): object to store unique blocks
        fp (float): float point threshold, default=0.01
        sim (float): block similarity threshold, ranges from 0 to 1, default=0.7
        stop_acc_drop (float): hard stop to deduplication when accuracy reduction is 
                               larger than the value, ranges from 0 to 1, default=0.04 
        eval_step (int): evaluate the model on evaluation dataset after k steps, defaule=5
        finetune (boolean): whether fine-tune after each deduplication
        x_train (numpy array): fine-tune dataset features matrix
        y_train (numpy array): fine-tune dataset labels
        ft_epochs (int): number of epochs for each fine-tune, default=1
        ft_batch_size (int): batch size of fine-tune, default=128
        ft_step (int): fine-tune the step after k step, default=5
        use_lsh (boolean): if it is enabled, then use lsh to search the most
                           similar block, other run a pairwise search. default=False

    """

    block_size_x = indexer.block_size_x
    block_size_y = indexer.block_size_y
    block_cap = block_size_x*block_size_y
    block_magnitudes = model_storage['block_magnitude']

    m_old = None
    m_old_ms = None
    df_old = None 

    if is_updated_model:
        assert path_to_ori_model is not None and path_to_ori_output is not None
        m_old = tf.keras.models.load_model(path_to_ori_model, custom_objects={'KerasLayer':hub.KerasLayer})
        m_old_ms = blocker.block_model_2d(m_old, block_size_x=block_size_x, block_size_y=block_size_y)
        import re
        def extract_coor(text):
            m = re.match(r"\((\d+),\s(\d+),\s(\d+)\)", text)
            return int(m.group(1)), int(m.group(2)), int(m.group(3))

        df_old = pd.read_csv(path_to_ori_output)
        df_old['duplicate_block_idx'] = df_old['duplicate_block_idx'].apply(lambda x: extract_coor(x))
    # Init results storage
    list_b1_index = []
    list_b2_index = []
    list_b2_sim = []
    list_acc = []
    list_is_deduplicate = []
    list_search_time = []
    list_total_time = []
    list_eval_time = []
    list_is_padded_block = []
    list_block_magnitude = []
    
    list_dedup_blocks_idx = []
    list_dedup_weights = []

    last_eval_step = 0

    # Obtain original accuracy
    if x_test is not None:
        _, ori_acc = model.evaluate(x_test, y_test, batch_size=eval_batch_size)
    if ds_test is not None:
        _, _, ori_acc = model.evaluate(ds_test, steps=ds_test_steps)

    step = 1

    # Init some timers
    timer_total = my_timer.Timer()
    timer_search = my_timer.Timer()
    timer_eval = my_timer.Timer()

    lsh_indexer = None
    # Build lsh index if lsh enabled
    if use_lsh:
        timer_lsh_index_build = my_timer.Timer()
        timer_lsh_index_build.tic()
        # Build LSH index
        lsh_indexer = l2lsh.L2LSH(prob_dim=block_cap, r=0.09, num_k=1, num_l=90, seed=0)
        for idx in tqdm(range(int(indexer.num_total)), leave=False):
            b = indexer.list_blocks[idx]
            b = b.reshape(-1)
            lsh_indexer.insert(b, idx)

        t_build_lsh_index = timer_lsh_index_build.toc()

        print('LSH index build time: {} ms'.format(t_build_lsh_index))


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
        # FIXME 999 is for L2
        # max_sim = 999  
        max_sim = 0
        # Index of the most similar block
        max_b2_index = ''  
        # Timer
        t_search_time = ''
        t_eval_time = ''
        t_total_time = ''
        is_padded_block = False
        is_padded_bias = False

        # check whether this is a padded block
        print(weight_idx)
        if  (len(model_storage['weights_shape'][weight_idx]) == 1 and block_size_x*block_size_y > 1):
            is_padded_block = True 
            is_padded_bias = True
        else:
            if (block_x_idx+1)*block_size_x > model_storage['weights_shape'][weight_idx][0] or \
            (block_y_idx+1)*block_size_y > model_storage['weights_shape'][weight_idx][1]:
                is_padded_block = True

        
        # Here is a trick: do not deduplicate the layer that has only 1 block will
        # gain better compression ratio and less accuracy reduction.
        if model_storage['weights_block_num'][weight_idx][0]*model_storage['weights_block_num'][weight_idx][1] > 1:
            # The block needs to be deduplicated
            b1 = model_storage['weights_padded'][weight_idx][block_x_idx*block_size_x: (
                block_x_idx+1)*block_size_x, block_y_idx*block_size_y: (block_y_idx+1)*block_size_y]

            # Tic for most similar block search
            timer_search.tic()
            query_result = None

            is_updated_block = True

            if is_updated_model:
                b1_old = m_old_ms['weights_padded'][weight_idx][block_x_idx*block_size_x: (
                    block_x_idx+1)*block_size_x, block_y_idx*block_size_y: (block_y_idx+1)*block_size_y]
                
                b1_sigs = lsh_indexer.compute_signatures_for_bands(b1)
                b1_old_sigs = lsh_indexer.compute_signatures_for_bands(b1_old)
                collide_match = 0
                
                for sig1, sig2 in zip(b1_sigs, b1_old_sigs):
                    if sig1 == sig2:
                        collide_match += 1
                
                if collide_match > 0: # threshold
                    is_updated_block = False

            if is_updated_block:
                if use_lsh:
                    # LSH search
                    query_result = lsh_indexer.query(b1)
                else:
                    # Pairwise search
                    query_result = range(indexer.num_total)
            else:
                # use old one to replace
                tuple_index = df_old[df_old['duplicate_block_idx'] == (weight_idx, block_x_idx, block_y_idx)].index
                find_new_one = True
                if len(tuple_index) != 0:
                    if not np.isnan(df_old.loc[tuple_index[0], 'deduplicate_block_idx']):
                        find_new_one = False

                # print(tuple_index)
                if find_new_one:
                    # prev not deduplicated 
                    is_updated_block = True
                    is_prev_dedup = False
                    if use_lsh:
                        # LSH search
                        query_result = lsh_indexer.query(b1)
                    else:
                        # Pairwise search
                        query_result = range(indexer.num_total)
                else:
                    print(b1_index)
                    is_prev_dedup = True
                    tuple_index = tuple_index[0]
                    query_result = [int(df_old.loc[tuple_index, 'deduplicate_block_idx'])]

            for b2_index in tqdm(query_result, leave=False):
                if b1_index == b2_index:
                    continue
                b2 = indexer.list_blocks[b2_index]

                # compute the similarity between a candidate block and the query block
                if not is_padded_block:
                    # FIXME
                    diff = np.abs(b1-b2)
                    block_sim = np.sum(diff <= fp) / block_cap          
                    # block_sim = np.linalg.norm(b1-b2)    
                else:
                    # if is padded block 
                    block_x_range_begin = block_x_idx*block_size_x
                    block_y_range_begin = block_y_idx*block_size_y
                    block_x_range_end = (block_x_idx+1)*block_size_x
                    block_y_range_end = (block_y_idx+1)*block_size_y
                    # resize the block
                    if block_x_range_end > model_storage['weights_shape'][weight_idx][0]:
                        block_x_range_end = model_storage['weights_shape'][weight_idx][0]
                    # TODO temp fix for bias block
                    if is_padded_bias:
                        block_y_range_end = 1
                    else:
                        if block_y_range_end > model_storage['weights_shape'][weight_idx][1]:
                            block_y_range_end = model_storage['weights_shape'][weight_idx][1]

                    block_x_length = block_x_range_end - block_x_range_begin
                    block_y_length = block_y_range_end - block_y_range_begin

                    b1_unpadded = b1[:block_x_length, :block_y_length]
                    b2_unpadded = b2[:block_x_length, :block_y_length]

                    # FIXME Use L2 to measure distance
                    diff = np.abs(b1_unpadded-b2_unpadded)
                    block_sim = np.sum(diff <= fp) / (block_x_length*block_y_length)    
                    # block_sim = np.linalg.norm(b1_unpadded-b2_unpadded)

                # only compute the similarity for non-zero part
                # b2_non_zero_part = b2[b1_non_zero_index]
                # diff = np.abs(b2_non_zero_part-b1_non_zero_part)
                # block_sim = np.sum(diff <= fp) / non_zero_size


                # block_sim = np.linalg.norm(b1-b2)
                # FIXME different criteria for different distance measurement
                if block_sim > max_sim and block_sim >= sim:
                # if block_sim < max_sim and block_sim >= sim:
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

            if is_updated_block:
                step += 1
            
            if finetune:
                list_dedup_blocks_idx.append((weight_idx, block_x_idx, block_y_idx))
                list_dedup_weights.append(max_b2)

            
        # Evaluation at each eval_step
        if step % eval_step == 0 and has_dedup:
            timer_eval.tic()
            if x_test is not None:
                _, acc = model.evaluate(x_test, y_test, batch_size=eval_batch_size)
            if ds_test is not None:
                _, _, acc = model.evaluate(ds_test, steps=ds_test_steps)
            t_eval_time = timer_eval.toc()
            last_eval_step = len(list_b1_index) + 1
            step += 1
        else:
            if len(list_acc) != 0:
                acc = list_acc[-1]
            else:
                acc = ori_acc
        if finetune and step % ft_step == 0:
            model, model_storage = finetune_model(model, model_storage, list_dedup_blocks_idx, list_dedup_weights,
                                                  x_train, y_train, block_size_x, block_size_y, epochs=ft_epochs, batch_size=ft_batch_size)
            if x_test is not None:
                _, acc = model.evaluate(x_test, y_test, batch_size=eval_batch_size)
            if ds_test is not None:
                _, _, acc = model.evaluate(ds_test, steps=ds_test_steps)
        
        t_total_time = timer_total.toc()

        list_b1_index.append((weight_idx, block_x_idx, block_y_idx))
        list_b2_index.append(max_b2_index)
        list_b2_sim.append(max_sim)
        list_acc.append(acc)
        list_is_deduplicate.append(has_dedup)
        list_search_time.append(t_search_time)
        list_total_time.append(t_total_time)
        list_eval_time.append(t_eval_time)
        list_is_padded_block.append(is_padded_block)
        list_block_magnitude.append(magnitude_v)

        print('Progress: ', len(list_b2_index), '/',
                len(block_magnitudes), ' Max sim', max_sim, 'Duplicate block ', magnitude_k, 
                ' Is Padded ',  is_padded_block, ' Block Magnitude ', magnitude_v)
    
        # Stop the deduplication once the accuracy reduction is larger than a threshold
        if ori_acc - acc > stop_acc_drop:
            break
    
    # perform a final evaluation in case there are blocks get deduplicated but the 
    # model is not evaluated

    if x_test is not None:
        _, acc = model.evaluate(x_test, y_test, batch_size=eval_batch_size)
    if ds_test is not None:
        _, _, acc = model.evaluate(ds_test, steps=ds_test_steps)

    if ori_acc - acc > stop_acc_drop:
        # need to revert to last dedup checkpoint 
        # num blocks needp to be reverted
        list_b1_index = list_b1_index[:last_eval_step]
        print(len(list_b1_index))
        list_b2_index = list_b2_index[:last_eval_step]
        list_b2_sim = list_b2_sim[:last_eval_step]
        list_acc = list_acc[:last_eval_step]
        list_is_deduplicate = list_is_deduplicate[:last_eval_step]
        list_search_time = list_search_time[:last_eval_step]
        list_total_time = list_total_time[:last_eval_step]
        list_eval_time = list_eval_time[:last_eval_step]
        list_is_padded_block = list_is_padded_block[:last_eval_step]
        list_block_magnitude = list_block_magnitude[:last_eval_step]
    else:
        # update the lastest acc
        list_acc[-1] = acc
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
                    list_is_padded_block.append('')
                    list_block_magnitude.append('')

    dedup_df = pd.DataFrame({'duplicate_block_idx': list_b1_index, 'deduplicate_block_idx': list_b2_index,
                                'block_similarity': list_b2_sim, 'model_accuracy': list_acc,
                                'is_padded_block': list_is_padded_block,
                                'is_deduplicated': list_is_deduplicate,
                                'block_magnitude': list_block_magnitude,
                                'search_time': list_search_time,
                                'eval_time': list_eval_time,
                                'total_time': list_total_time})
    dedup_df.head()

    return dedup_df

def generate_detector_output(df, ms, indexer_num_total):
    """Generate an output with given detector dataframe output

    Args:
        df (pd.Dataframe): deduplicate_model dataframe output
        ms (model storage): model storage
        indexer_num_total (int): number of unique blocks in current blocks indexer

    Returns:
        dict: detector output
        int: the number of unique blocks for given model after deduplication
    """
    tensor_dimension = ms['weights_block_num']
    list_tensor_mapping = []
    
    for _ in range(len(tensor_dimension)):
        list_tensor_mapping.append(dict())

    for index, row in df.iterrows():
        is_deduplicated = row['is_deduplicated']
        if is_deduplicated:
            tensor_idx, block_x_idx, block_y_idx = row['duplicate_block_idx']
            deduplicate_block_idx = int(row['deduplicate_block_idx'])
            list_tensor_mapping[tensor_idx][(block_x_idx, block_y_idx)] = deduplicate_block_idx
    
    num_new_unique_blocks = 0
    # create index for distinct block
    for tensor_index in range(len(tensor_dimension)):
        tensor_shape_x, tensor_shape_y = tensor_dimension[tensor_index]
        for i in range(tensor_shape_x):
            for j in range(tensor_shape_y):
                if (i, j) not in list_tensor_mapping[tensor_index].keys():
                    list_tensor_mapping[tensor_index][(i, j)] = indexer_num_total + num_new_unique_blocks
                    num_new_unique_blocks += 1
    
    list_distinct_blocks = set()
    for mapping in list_tensor_mapping:
        list_distinct_blocks.update(mapping.values())

    detector_output = dict()
    detector_output['tensor_mapping'] = list_tensor_mapping
    detector_output['blocked_tensor_dimension'] = tensor_dimension
    detector_output['num_distinct_blocks'] = len(list_distinct_blocks)
    return detector_output, indexer_num_total + num_new_unique_blocks