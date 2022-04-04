from scipy.sparse import data
import tensorflow_hub as hub
import tensorflow as tf
import numpy as np
import tensorflow_datasets as tfds
import pandas as pd
import downloader
import os
import model_trainer_sentiment_task as model_trainer
import deduplicator
import blocker
import indexer
from scipy.spatial import distance
from sklearn.model_selection import train_test_split
from tensorflow.keras import layers

def run_on_first_model():
    """Here, we use the w2v_wiki500_yelp_embed_nontrainable mdoel to build indexes
    of block then save it to disk for further usage.
    """
    model_path = 'models'

    # Define block size
    block_size_x = 10000
    block_size_y = 100

    # Build block indexes on w2v_wiki500_yelp_embed_nontrainable
    m1 = tf.keras.models.load_model(os.path.join(model_path, 'w2v_wiki500_yelp_embed_nontrainable.h5'), 
                                    custom_objects={'KerasLayer':hub.KerasLayer})
    m1_ms = blocker.block_model_2d(m1, block_size_x=block_size_x, block_size_y=block_size_y)

    # Create indexer
    blocks_indexer = indexer.Indexer(block_size_x=block_size_x, block_size_y=block_size_y)
    blocks_indexer.build_index(m1_ms, 'm1')

    blocks_indexer.save('blocks_indexer_1.npy')

def run_on_second_model(method):
    """Here, we profrm the deduplication to w2v_wiki500_imdb_embed_nontrainable model,
    it first reload the indexes pre-saved from the first step, then performing a deduplication.
    For the not-deduplicated blocks, it will be treated as unique blocks and gets updated to
    indexer and saved to disk.

    Please use the correct commented out code to evaluate different approaches.
    """
    model_path = 'models'

    # Define block size
    block_size_x = 10000
    block_size_y = 100

    # Create indexer
    blocks_indexer = indexer.Indexer(block_size_x=block_size_x, block_size_y=block_size_y)
    blocks_indexer.load('blocks_indexer_1.npy')

    # Load model
    m2 = tf.keras.models.load_model(os.path.join(model_path, 'w2v_wiki500_imdb_embed_nontrainable.h5'), 
                                    custom_objects={'KerasLayer':hub.KerasLayer})
    m2_ms = blocker.block_model_2d(m2, block_size_x=block_size_x, block_size_y=block_size_y)

    m2.compile(optimizer='adam', loss='binary_crossentropy', metrics=['accuracy'])
    # Load dataset for imdb
    x, y = model_trainer.load_dataset(dataset="imdb_reviews")
    x_train, x_test, y_train, y_test = train_test_split(
                                        x, y, test_size=0.2, random_state=0)

    if method == 'enhanced_pairwise':
        # Run deduplication (enhanced-pairwise)
        result_df = deduplicator.deduplicate_model(m2, m2_ms, x_test, y_test, 
                                    blocks_indexer, fp=0.01, sim=0.7, stop_acc_drop=0.035, 
                                    use_lsh=False, eval_step=500)
    elif method == 'proposed_wo_finetune':
        # Run deduplication (proposed w/o fine-tune)
        result_df = deduplicator.deduplicate_model(m2, m2_ms, x_test, y_test, 
                                    blocks_indexer, fp=0.01, sim=0, stop_acc_drop=0.035, 
                                    use_lsh=True)
    elif method == 'proposed_w_finetune':
        # Run deduplication (proposed w/ fine-tune)
        result_df = deduplicator.deduplicate_model(m2, m2_ms, x_test, y_test, 
                                    blocks_indexer, fp=0.01, sim=0.7, stop_acc_drop=0.035, 
                                    use_lsh=True, x_train=x_train, y_train=y_train,
                                    finetune=True)

    detector_output, num_unique_blocks = deduplicator.generate_detector_output(result_df, m2_ms, blocks_indexer.num_total)
    result_df.to_csv('2.csv', index=False)
    np.save('2.npy', detector_output)

    # Update index
    blocks_indexer.update_index(m2_ms, result_df)

    # Save to disk
    blocks_indexer.save('blocks_indexer_2.npy')

def run_on_third_model(method):
    """Here, we profrm the deduplication to w2v_wiki500_imdb_embed_trainable model,
    it first reload the indexes pre-saved from the first step, then performing a deduplication.
    For the not-deduplicated blocks, it will be treated as unique blocks and gets updated to
    indexer and saved to disk.

    Please use the correct commented out code to evaluate different approaches.
    """
    model_path = 'models'

    # Define block size
    block_size_x = 10000
    block_size_y = 100

    # Create indexer
    blocks_indexer = indexer.Indexer(block_size_x=block_size_x, block_size_y=block_size_y)
    blocks_indexer.load('blocks_indexer_2.npy')

    # Load model
    m2 = tf.keras.models.load_model(os.path.join(model_path, 'w2v_wiki500_imdb_embed_trainable.h5'), 
                                    custom_objects={'KerasLayer':hub.KerasLayer})
    m2_ms = blocker.block_model_2d(m2, block_size_x=block_size_x, block_size_y=block_size_y)

    m2.compile(optimizer='adam', loss='binary_crossentropy', metrics=['accuracy'])
    # Load dataset for imdb
    x, y = model_trainer.load_dataset(dataset="imdb_reviews")
    x_train, x_test, y_train, y_test = train_test_split(
                                        x, y, test_size=0.2, random_state=0)

    if method == 'enhanced_pairwise':
        # Run deduplication (enhanced-pairwise)
        result_df = deduplicator.deduplicate_model(m2, m2_ms, x_test, y_test, 
                                    blocks_indexer, fp=0.01, sim=0.7, stop_acc_drop=0.035, 
                                    use_lsh=False)
    elif method == 'proposed_wo_finetune':
        # Run deduplication (proposed w/o fine-tune)
        result_df = deduplicator.deduplicate_model(m2, m2_ms, x_test, y_test, 
                                    blocks_indexer, fp=0.01, sim=0, stop_acc_drop=0.035, 
                                    use_lsh=True)
    elif method == 'proposed_w_finetune':
        # Run deduplication (proposed w/ fine-tune)
        result_df = deduplicator.deduplicate_model(m2, m2_ms, x_test, y_test, 
                                    blocks_indexer, fp=0.01, sim=0.7, stop_acc_drop=0.035, 
                                    use_lsh=True, x_train=x_train, y_train=y_train,
                                    finetune=True)

    detector_output, num_unique_blocks = deduplicator.generate_detector_output(result_df, m2_ms, blocks_indexer.num_total)
    result_df.to_csv('3.csv', index=False)
    np.save('3.npy', detector_output)

    # Update index
    blocks_indexer.update_index(m2_ms, result_df)

    # Save to disk
    blocks_indexer.save('blocks_indexer_3.npy')

def run_on_fourth_model(method):
    """Here, we profrm the deduplication to w2v_wiki500_yelp_embed_trainable model,
    it first reload the indexes pre-saved from the first step, then performing a deduplication.
    For the not-deduplicated blocks, it will be treated as unique blocks and gets updated to
    indexer and saved to disk.

    Please use the correct commented out code to evaluate different approaches.
    """
    model_path = 'models'

    # Define block size
    block_size_x = 10000
    block_size_y = 100

    # Create indexer
    blocks_indexer = indexer.Indexer(block_size_x=block_size_x, block_size_y=block_size_y)
    blocks_indexer.load('blocks_indexer_3.npy')

    # Load model
    m2 = tf.keras.models.load_model(os.path.join(model_path, 'w2v_wiki500_yelp_embed_trainable.h5'), 
                                    custom_objects={'KerasLayer':hub.KerasLayer})
    m2_ms = blocker.block_model_2d(m2, block_size_x=block_size_x, block_size_y=block_size_y)

    m2.compile(optimizer='adam', loss='binary_crossentropy', metrics=['accuracy'])
    # Load dataset for yelp
    x, y = model_trainer.load_dataset(dataset="yelp_reviews")
    x_train, x_test, y_train, y_test = train_test_split(
                                        x, y, test_size=0.2, random_state=0)

    if method == 'enhanced_pairwise':
        # Run deduplication (enhanced-pairwise)
        result_df = deduplicator.deduplicate_model(m2, m2_ms, x_test, y_test, 
                                    blocks_indexer, fp=0.01, sim=0.7, stop_acc_drop=0.035, 
                                    use_lsh=False)
    elif method == 'proposed_wo_finetune':
        # Run deduplication (proposed w/o fine-tune)
        result_df = deduplicator.deduplicate_model(m2, m2_ms, x_test, y_test, 
                                    blocks_indexer, fp=0.01, sim=0, stop_acc_drop=0.035, 
                                    use_lsh=True)
    elif method == 'proposed_w_finetune':
        # Run deduplication (proposed w/ fine-tune)
        result_df = deduplicator.deduplicate_model(m2, m2_ms, x_test, y_test, 
                                    blocks_indexer, fp=0.01, sim=0.7, stop_acc_drop=0.035, 
                                    use_lsh=True, x_train=x_train, y_train=y_train,
                                    finetune=True)

    detector_output, num_unique_blocks = deduplicator.generate_detector_output(result_df, m2_ms, blocks_indexer.num_total)
    result_df.to_csv('4.csv', index=False)
    np.save('4.npy', detector_output)

    # Update index
    blocks_indexer.update_index(m2_ms, result_df)

    # Save to disk
    blocks_indexer.save('blocks_indexer_4.npy')


def run_on_fifth_model(method):
    """Here, we profrm the deduplication to w2v_wiki500_civil_comment_embed_trainable model,
    it first reload the indexes pre-saved from the first step, then performing a deduplication.
    For the not-deduplicated blocks, it will be treated as unique blocks and gets updated to
    indexer and saved to disk.

    Please use the correct commented out code to evaluate different approaches.
    """
    model_path = 'models'

    # Define block size
    block_size_x = 10000
    block_size_y = 100

    # Create indexer
    blocks_indexer = indexer.Indexer(block_size_x=block_size_x, block_size_y=block_size_y)
    blocks_indexer.load('blocks_indexer_4.npy')

    # Load model
    m2 = tf.keras.models.load_model(os.path.join(model_path, 'w2v_wiki500_civil_comment_embed_trainable.h5'), 
                                    custom_objects={'KerasLayer':hub.KerasLayer})
    m2_ms = blocker.block_model_2d(m2, block_size_x=block_size_x, block_size_y=block_size_y)

    m2.compile(optimizer='adam', loss='binary_crossentropy', metrics=['accuracy'])
    # Load dataset for civil
    x, y = model_trainer.load_dataset(dataset="civil_comment")
    x_train, x_test, y_train, y_test = train_test_split(
                                        x, y, test_size=0.2, random_state=0)

    if method == 'enhanced_pairwise':
        # Run deduplication (enhanced-pairwise)
        result_df = deduplicator.deduplicate_model(m2, m2_ms, x_test, y_test, 
                                    blocks_indexer, fp=0.01, sim=0, stop_acc_drop=0.035, 
                                    use_lsh=False)
    elif method == 'proposed_wo_finetune':
        # Run deduplication (proposed w/o fine-tune)
        result_df = deduplicator.deduplicate_model(m2, m2_ms, x_test, y_test, 
                                    blocks_indexer, fp=0.01, sim=0, stop_acc_drop=0.035, 
                                    use_lsh=True)
    elif method == 'proposed_w_finetune':
        # Run deduplication (proposed w/ fine-tune)
        result_df = deduplicator.deduplicate_model(m2, m2_ms, x_test, y_test, 
                                    blocks_indexer, fp=0.01, sim=0.7, stop_acc_drop=0.035, 
                                    use_lsh=True, x_train=x_train, y_train=y_train,
                                    finetune=True)

    detector_output, num_unique_blocks = deduplicator.generate_detector_output(result_df, m2_ms, blocks_indexer.num_total)
    result_df.to_csv('5.csv', index=False)
    np.save('5.npy', detector_output)

    # Update index
    blocks_indexer.update_index(m2_ms, result_df)

    # Save to disk
    blocks_indexer.save('blocks_indexer_5.npy')



def main():
    # Run on CPU/GPU
    run_on_cpu = True
    model_trainer.run_device(run_on_cpu=run_on_cpu)
    # Method: enhanced_pairwise, proposed_wo_finetune, or proposed_w_finetune
    method = 'proposed_wo_finetune'
    run_on_first_model()
    run_on_second_model(method)
    run_on_third_model(method)
    run_on_fourth_model(method)
    run_on_fifth_model(method)

    
if __name__ == "__main__":
    main()