from scipy.sparse import data
import tensorflow_hub as hub
import tensorflow as tf
import numpy as np
import tensorflow_datasets as tfds
import pandas as pd
import downloader
import os
import model_trainer_sentiment_task
import deduplicator
import blocker
import indexer
from scipy.spatial import distance
from sklearn.model_selection import train_test_split


def run_on_model_1():
    """Here, we use the rcv1_2k_xml mdoel to build indexes
    of block then save it to disk for further usage.
    """
    model_path = 'models'

    # Define block size
    block_size_x = 10000
    block_size_y = 50

    # Build block indexes on rcv1_2k_xml
    m1 = tf.keras.models.load_model(os.path.join(model_path, 'rcv1_2k_xml.h5'))
    m1_ms = blocker.block_model_2d(m1, block_size_x=block_size_x, block_size_y=block_size_y)


    # Create indexer
    blocks_indexer = indexer.Indexer(block_size_x=block_size_x, block_size_y=block_size_y)
    blocks_indexer.build_index(m1_ms, 'm1')

    blocks_indexer.save('hetero_model_exp3_blocks_indexer_1.npy')


def run_on_model_2(method):
    """Here, we perform the deduplication to nnlm_en_dim128_yelp_embed_trainable model,
    it first reload the indexes pre-saved from the first step, then performing a deduplication.
    For the not-deduplicated blocks, it will be treated as unique blocks and gets updated to
    indexer and saved to disk.

    Please use the correct commented out code to evaluate different approaches.
    """
    model_path = 'models'

    # Define block size
    block_size_x = 10000
    block_size_y = 50

    # Create indexer
    blocks_indexer = indexer.Indexer(block_size_x=block_size_x, block_size_y=block_size_y)
    blocks_indexer.load('hetero_model_exp3_blocks_indexer_1.npy')

    
    m2 = tf.keras.models.load_model(os.path.join(model_path, 'nnlm_en_dim128_yelp_embed_trainable.h5'), 
                                    custom_objects={'KerasLayer':hub.KerasLayer})
    m2_ms = blocker.block_model_2d(m2, block_size_x=block_size_x, block_size_y=block_size_y)

    m2.compile(optimizer='adam', loss='binary_crossentropy', metrics=['accuracy'])
    # Load dataset for imdb
    x, y = model_trainer_sentiment_task.load_dataset(dataset="yelp_reviews")
    x_train, x_test, y_train, y_test = train_test_split(
                                        x, y, test_size=0.2, random_state=0)

    if method == 'enhanced_pairwise':
        # Run deduplication (enhanced-pairwise)
        result_df = deduplicator.deduplicate_model(m2, m2_ms, x_test, y_test, 
                                    blocks_indexer, fp=0.01, sim=0.1, stop_acc_drop=0.035, 
                                    use_lsh=False)
    elif method == 'proposed_wo_finetune':
        # Run deduplication (proposed w/o fine-tune)
        result_df = deduplicator.deduplicate_model(m2, m2_ms, x_test, y_test, 
                                    blocks_indexer, fp=0.01, sim=0, stop_acc_drop=0.035, 
                                    use_lsh=True)
    elif method == 'proposed_w_finetune':
        # Run deduplication (proposed w/ fine-tune)
        result_df = deduplicator.deduplicate_model(m2, m2_ms, x_test, y_test, 
                                    blocks_indexer, fp=0.01, sim=0.1, stop_acc_drop=0.035, 
                                    use_lsh=True, x_train=x_train, y_train=y_train,
                                    finetune=True)

    detector_output, num_unique_blocks = deduplicator.generate_detector_output(result_df, m2_ms, blocks_indexer.num_total)
    result_df.to_csv('hetero_model_exp3_2.csv', index=False)
    np.save('hetero_model_exp3_2', detector_output)

    # Update index
    blocks_indexer.update_index(m2_ms, result_df)

    # Save to disk
    blocks_indexer.save('hetero_model_exp3_blocks_indexer_1.npy')


def main():
    # Run on CPU/GPU
    run_on_cpu = False
    model_trainer_sentiment_task.run_device(run_on_cpu=run_on_cpu)
    # Method: enhanced_pairwise, proposed_wo_finetune, or proposed_w_finetune
    method = 'proposed_wo_finetune'
    run_on_model_1()
    run_on_model_2(method)

    
if __name__ == "__main__":
    main()