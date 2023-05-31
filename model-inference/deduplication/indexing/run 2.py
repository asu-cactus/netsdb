from scipy.sparse import data
import tensorflow_hub as hub
import tensorflow as tf
import numpy as np
import tensorflow_datasets as tfds
import pandas as pd
import downloader
import os
import model_trainer
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
    blocks_indexer.build_index(m1_ms, 'yelp_embed_nontrainable')

    blocks_indexer.save('blocks_indexer.npy')

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
    blocks_indexer.load('blocks_indexer.npy')

    # Run deduplication on w2v_wiki500_imdb_embed_nontrainable
    # Load model
    m2 = tf.keras.models.load_model(os.path.join(model_path, 'w2v_wiki500_imdb_embed_nontrainable.h5'), 
                                    custom_objects={'KerasLayer':hub.KerasLayer})
    m2_ms = blocker.block_model_2d(m2, block_size_x=block_size_x, block_size_y=block_size_y)

    # Load dataset for imdb
    imdb_x, imdb_y = model_trainer.load_dataset(dataset="imdb_reviews")
    imdb_x_train, imdb_x_test, imdb_y_train, imdb_y_test = train_test_split(
                                        imdb_x, imdb_y, test_size=0.2, random_state=0)

    if method == 'enhanced_pairwise':
        # Run deduplication (enhanced-pairwise)
        result_df = deduplicator.deduplicate_model(m2, m2_ms, imdb_x_test, imdb_y_test, 
                                    blocks_indexer, fp=0.01, sim=0.7, stop_acc_drop=0.035, 
                                    use_lsh=False)
    elif method == 'proposed_wo_finetune':
        # Run deduplication (proposed w/o fine-tune)
        result_df = deduplicator.deduplicate_model(m2, m2_ms, imdb_x_test, imdb_y_test, 
                                    blocks_indexer, fp=0.01, sim=0.7, stop_acc_drop=0.035, 
                                    use_lsh=True)
    elif method == 'proposed_w_finetune':
        # Run deduplication (proposed w/ fine-tune)
        result_df = deduplicator.deduplicate_model(m2, m2_ms, imdb_x_test, imdb_y_test, 
                                    blocks_indexer, fp=0.01, sim=0.7, stop_acc_drop=0.035, 
                                    use_lsh=True, x_train=imdb_x_train, y_train=imdb_y_train,
                                    finetune=True)

    detector_output, num_unique_blocks = deduplicator.generate_detector_output(result_df, m2_ms, blocks_indexer.num_total)
    result_df.to_csv('imdb_embed_nontrainable_result.csv', index=False)
    np.save('imdb_embed_nontrainable_output.npy', detector_output)

    # Update index
    blocks_indexer.update_index(m2_ms, result_df)

    # Save to disk
    blocks_indexer.save('blocks_indexer.npy')


def main():
    # Run on CPU/GPU
    run_on_cpu = False
    model_trainer.run_device(run_on_cpu=run_on_cpu)
    # Method: enhanced_pairwise, proposed_wo_finetune, or proposed_w_finetune
    method = 'enhanced_pairwise'
    run_on_first_model()
    run_on_second_model(method)
    
if __name__ == "__main__":
    main()