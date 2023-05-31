from scipy.sparse import data
import tensorflow_hub as hub
import tensorflow as tf
import numpy as np
import tensorflow_datasets as tfds
import pandas as pd
import downloader
import os
import model_trainer_extreme_task
import deduplicator
import blocker
import indexer
from scipy.spatial import distance
from sklearn.model_selection import train_test_split
from tensorflow.keras import layers

def dedup_model1():
    """Here, we use the amazoncat_13k_xml mdoel to build indexes
    of block then save it to disk for further usage.
    """
    model_path = 'models'

    # Define block size
    block_size_x = 10000
    block_size_y = 50

    # Build block indexes on amazoncat_13k_xml
    m1 = tf.keras.models.load_model(os.path.join(model_path, 'amazoncat_13k_xml.h5'))
    m1_ms = blocker.block_model_2d(m1, block_size_x=block_size_x, block_size_y=block_size_y)

    # Create indexer
    blocks_indexer = indexer.Indexer(block_size_x=block_size_x, block_size_y=block_size_y)
    blocks_indexer.build_index(m1_ms, 'm1')
    
    blocks_indexer.save('exp_norm_blocks_indexer_1.npy')

def dedup_model2_norm_by_layer_dedup_by_layer(method):
    """Here, we perform the deduplication to rcv1_2k_xml model, while we normalize the block magnitude
    by layer then order them by layer (largest first)

    Please use the correct commented out code to evaluate different approaches.
    """
    model_path = 'models'

    # Define block size
    block_size_x = 10000
    block_size_y = 50

    # Create indexer
    blocks_indexer = indexer.Indexer(block_size_x=block_size_x, block_size_y=block_size_y)
    blocks_indexer.load('exp_norm_blocks_indexer_1.npy')

    m2 = tf.keras.models.load_model(os.path.join(model_path, 'rcv1_2k_xml.h5'))
    m2_ms = blocker.block_model_2d(m2, block_size_x=block_size_x, block_size_y=block_size_y, 
                                    norm_by_layer=True, dedup_by_layer=True)

    optimizer = tf.keras.optimizers.Adam(learning_rate=1e-3, decay=1e-3/100)
    m2.compile(optimizer=optimizer, loss=tf.keras.losses.BinaryCrossentropy(from_logits=True), metrics=['accuracy', model_trainer_extreme_task.PK1_Metric])
    m2.run_eagerly = True
    tf.executing_eagerly()

    batch_size=512
    ds_train, ds_test, num_train, num_test, num_features, num_labels = model_trainer_extreme_task.load_dataset(dataset="rcv1-2k", batch_size=batch_size)
    ds_test_steps = np.ceil(num_test/batch_size)

    if method == 'enhanced_pairwise':
        # Run deduplication (enhanced-pairwise)
        result_df = deduplicator.deduplicate_model(m2, m2_ms, ds_test=ds_test, 
                                    indexer=blocks_indexer, fp=0.01, sim=0.05, stop_acc_drop=0.035, 
                                    use_lsh=False, ds_test_steps=ds_test_steps, eval_step=10)
    elif method == 'proposed_wo_finetune':
        # Run deduplication (proposed w/o fine-tune)
        result_df = deduplicator.deduplicate_model(m2, m2_ms, ds_test=ds_test, 
                                    indexer=blocks_indexer, fp=0.01, sim=0.05, stop_acc_drop=0.035, 
                                    use_lsh=True, ds_test_steps=ds_test_steps, eval_step=10)

    detector_output, num_unique_blocks = deduplicator.generate_detector_output(result_df, m2_ms, blocks_indexer.num_total)
    result_df.to_csv('exp_norm_m2_norm_by_layer_dedup_by_layer.csv', index=False)
    np.save('exp_norm_m2_norm_by_layer_dedup_by_layer.npy', detector_output)

def dedup_model2_norm_by_layer_dedup_together(method):
    """Here, we perform the deduplication to rcv1_2k_xml model, while we normalize the block magnitude
    by layer then order them together

    Please use the correct commented out code to evaluate different approaches.
    """
    model_path = 'models'

    # Define block size
    block_size_x = 10000
    block_size_y = 50

    # Create indexer
    blocks_indexer = indexer.Indexer(block_size_x=block_size_x, block_size_y=block_size_y)
    blocks_indexer.load('exp_norm_blocks_indexer_1.npy')

    m2 = tf.keras.models.load_model(os.path.join(model_path, 'rcv1_2k_xml.h5'))
    m2_ms = blocker.block_model_2d(m2, block_size_x=block_size_x, block_size_y=block_size_y, 
                                    norm_by_layer=True, dedup_by_layer=False)

    optimizer = tf.keras.optimizers.Adam(learning_rate=1e-3, decay=1e-3/100)
    m2.compile(optimizer=optimizer, loss=tf.keras.losses.BinaryCrossentropy(from_logits=True), metrics=['accuracy', model_trainer_extreme_task.PK1_Metric])
    m2.run_eagerly = True
    tf.executing_eagerly()

    batch_size=512
    ds_train, ds_test, num_train, num_test, num_features, num_labels = model_trainer_extreme_task.load_dataset(dataset="rcv1-2k", batch_size=batch_size)
    ds_test_steps = np.ceil(num_test/batch_size)

    if method == 'enhanced_pairwise':
        # Run deduplication (enhanced-pairwise)
        result_df = deduplicator.deduplicate_model(m2, m2_ms, ds_test=ds_test, 
                                    indexer=blocks_indexer, fp=0.01, sim=0.05, stop_acc_drop=0.035, 
                                    use_lsh=False, ds_test_steps=ds_test_steps, eval_step=10)
    elif method == 'proposed_wo_finetune':
        # Run deduplication (proposed w/o fine-tune)
        result_df = deduplicator.deduplicate_model(m2, m2_ms, ds_test=ds_test, 
                                    indexer=blocks_indexer, fp=0.01, sim=0.05, stop_acc_drop=0.035, 
                                    use_lsh=True, ds_test_steps=ds_test_steps, eval_step=10)

    detector_output, num_unique_blocks = deduplicator.generate_detector_output(result_df, m2_ms, blocks_indexer.num_total)
    result_df.to_csv('exp_norm_m2_norm_by_layer_dedup_together.csv', index=False)
    np.save('exp_norm_m2_norm_by_layer_dedup_together.npy', detector_output)

def dedup_model2_no_norm_dedup_by_layer(method):
    """Here, we perform the deduplication to rcv1_2k_xml model, while we do not normalize the block magnitude
    then order them by layer (largest first)

    Please use the correct commented out code to evaluate different approaches.
    """
    model_path = 'models'

    # Define block size
    block_size_x = 10000
    block_size_y = 50

    # Create indexer
    blocks_indexer = indexer.Indexer(block_size_x=block_size_x, block_size_y=block_size_y)
    blocks_indexer.load('exp_norm_blocks_indexer_1.npy')

    m2 = tf.keras.models.load_model(os.path.join(model_path, 'rcv1_2k_xml.h5'))
    m2_ms = blocker.block_model_2d(m2, block_size_x=block_size_x, block_size_y=block_size_y, 
                                    norm_by_layer=False, dedup_by_layer=True)

    optimizer = tf.keras.optimizers.Adam(learning_rate=1e-3, decay=1e-3/100)
    m2.compile(optimizer=optimizer, loss=tf.keras.losses.BinaryCrossentropy(from_logits=True), metrics=['accuracy', model_trainer_extreme_task.PK1_Metric])
    m2.run_eagerly = True
    tf.executing_eagerly()

    batch_size=512
    ds_train, ds_test, num_train, num_test, num_features, num_labels = model_trainer_extreme_task.load_dataset(dataset="rcv1-2k", batch_size=batch_size)
    ds_test_steps = np.ceil(num_test/batch_size)

    if method == 'enhanced_pairwise':
        # Run deduplication (enhanced-pairwise)
        result_df = deduplicator.deduplicate_model(m2, m2_ms, ds_test=ds_test, 
                                    indexer=blocks_indexer, fp=0.01, sim=0.05, stop_acc_drop=0.035, 
                                    use_lsh=False, ds_test_steps=ds_test_steps, eval_step=10)
    elif method == 'proposed_wo_finetune':
        # Run deduplication (proposed w/o fine-tune)
        result_df = deduplicator.deduplicate_model(m2, m2_ms, ds_test=ds_test, 
                                    indexer=blocks_indexer, fp=0.01, sim=0.05, stop_acc_drop=0.035, 
                                    use_lsh=True, ds_test_steps=ds_test_steps, eval_step=10)

    detector_output, num_unique_blocks = deduplicator.generate_detector_output(result_df, m2_ms, blocks_indexer.num_total)
    result_df.to_csv('exp_norm_m2_no_norm_dedup_by_layer.csv', index=False)
    np.save('exp_norm_m2_no_norm_dedup_by_layer.npy', detector_output)

def dedup_model2_no_norm_dedup_together(method):
    """Here, we perform the deduplication to rcv1_2k_xml model, while we do not normalize the block magnitude
    then order them together(largest first)

    Please use the correct commented out code to evaluate different approaches.
    """
    model_path = 'models'

    # Define block size
    block_size_x = 10000
    block_size_y = 50

    # Create indexer
    blocks_indexer = indexer.Indexer(block_size_x=block_size_x, block_size_y=block_size_y)
    blocks_indexer.load('exp_norm_blocks_indexer_1.npy')

    m2 = tf.keras.models.load_model(os.path.join(model_path, 'rcv1_2k_xml.h5'))
    m2_ms = blocker.block_model_2d(m2, block_size_x=block_size_x, block_size_y=block_size_y, 
                                    norm_by_layer=False, dedup_by_layer=False)

    optimizer = tf.keras.optimizers.Adam(learning_rate=1e-3, decay=1e-3/100)
    m2.compile(optimizer=optimizer, loss=tf.keras.losses.BinaryCrossentropy(from_logits=True), metrics=['accuracy', model_trainer_extreme_task.PK1_Metric])
    m2.run_eagerly = True
    tf.executing_eagerly()

    batch_size=512
    ds_train, ds_test, num_train, num_test, num_features, num_labels = model_trainer_extreme_task.load_dataset(dataset="rcv1-2k", batch_size=batch_size)
    ds_test_steps = np.ceil(num_test/batch_size)

    if method == 'enhanced_pairwise':
        # Run deduplication (enhanced-pairwise)
        result_df = deduplicator.deduplicate_model(m2, m2_ms, ds_test=ds_test, 
                                    indexer=blocks_indexer, fp=0.01, sim=0.05, stop_acc_drop=0.035, 
                                    use_lsh=False, ds_test_steps=ds_test_steps, eval_step=10)
    elif method == 'proposed_wo_finetune':
        # Run deduplication (proposed w/o fine-tune)
        result_df = deduplicator.deduplicate_model(m2, m2_ms, ds_test=ds_test, 
                                    indexer=blocks_indexer, fp=0.01, sim=0.05, stop_acc_drop=0.035, 
                                    use_lsh=True, ds_test_steps=ds_test_steps, eval_step=10)

    detector_output, num_unique_blocks = deduplicator.generate_detector_output(result_df, m2_ms, blocks_indexer.num_total)
    result_df.to_csv('exp_norm_m2_no_norm_dedup_together.csv', index=False)
    np.save('exp_norm_m2_no_norm_dedup_by_together.npy', detector_output)


def main():
    # Run on CPU/GPU
    run_on_cpu = True
    model_trainer_extreme_task.run_device(run_on_cpu=run_on_cpu)
    # Method: enhanced_pairwise, proposed_wo_finetune, or proposed_w_finetune
    method = 'proposed_wo_finetune'
    dedup_model1()
    dedup_model2_norm_by_layer_dedup_by_layer(method)
    dedup_model2_norm_by_layer_dedup_together(method)
    dedup_model2_no_norm_dedup_by_layer(method)
    dedup_model2_no_norm_dedup_together(method)
    
if __name__ == "__main__":
    main()