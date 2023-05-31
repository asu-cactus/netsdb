from scipy.sparse import data
import tensorflow_hub as hub
import tensorflow as tf
import numpy as np
import tensorflow_datasets as tfds
import pandas as pd
import downloader
import os
from scipy.spatial import distance
from sklearn.model_selection import train_test_split
from tensorflow.keras import layers


def run_device(run_on_cpu):
    if run_on_cpu:
        os.environ['CUDA_VISIBLE_DEVICES'] = '-1'


def obtain_dataset_info(path_to_train, path_to_test):
    """ Obtain the dataset info from the given path, the data info is stored in the
    first line

    Args:
        path_to_train (str): path to the training data
        path_to_test (str): path to the testing data 
    
    Returns:
        (int, int, int, int): number of training data, number of testing data, 
                              number of features, number of labels
    """
    with open(path_to_train) as f:
        line = f.readline()
    num_train, num_features, num_labels = line.split(' ')

    with open(path_to_test) as f:
        line = f.readline()
    num_test, _, _ = line.split(' ')

    return int(num_train), int(num_test), int(num_features), int(num_labels)


def parse_function(data, num_features, num_labels):
    """ Parse each row (sparse representation) to a feature vector

    Args:
        data: each row in the text file
        num_features (int): number of features 
        num_labels (int): number of labels  
    
    Returns:
        numpy array, numpy array: feature vector, multi-label vector
    """
    features = np.zeros(num_features, dtype=np.float32)
    record = data.decode('utf-8')
    # return record
    elements = record.split(' ')
    labels = [int(val) for val in elements[0].split(',')]
    labels = tf.reduce_sum(tf.one_hot(labels, depth=num_labels), axis=0)
    for ft_pairs in elements[1:]:
        ft, ft_val = ft_pairs.split(':')
        features[int(ft)] = float(ft_val)
    labels = tf.cast(labels, tf.bool)
    return features, labels


@tf.function()
def tf_function(input, num_features, num_labels):
    """ A tf wrapper to parse data for tensorflow dataset 

    Args:
        input: each row in the text file 
        num_features (int): number of features 
        num_labels (int): number of labels  
    
    Returns:
        (numpy array, numpy_array): parsed feature vector, parsed multi-label vector
    """
    x, y = tf.numpy_function(parse_function, [input, num_features, num_labels], (tf.float32, tf.bool))
    x.set_shape([num_features])
    y.set_shape([num_labels])
    return x, y


def PK1_Metric(y_true, y_pred):
    """ Compute Precision on Top 1 Prediction

    Args:
        y_ture (numpy array): ground truth 
        y_pred (numpy array): prediction 
    
    Returns:
        float: Precision on Top 1 Prediction
    """
    topK = 1
    y_pred_indices = np.argsort(y_pred.numpy(), axis=1)[:, -topK:]
    y_true = y_true.numpy()
    topK_hit = 0
    for i, l in enumerate(y_pred_indices):
        topK_hit += sum(y_true[i, y_pred_indices[i]])
    acc = topK_hit / (len(y_true)*topK)
    return acc

def load_dataset(dataset, batch_size=512):
    """Load dataset with given dataset name.
    Args:
        dataset (str): name of the dataset, it has to be amazoncat-13k, amazoncat-14k,
                          eurlex-4.3k or rcv1-2k 
        batch_size (int): batch size of tf dataset 

    Returns:
        (tf.dataset, tf.dataset, int, int, int, int): training dataset, testing dataset,
                                                      number of training data, number of testing data, 
                                                      number of features, number of labels
    """
    if dataset not in ['amazoncat-13k', 'amazoncat-14k', 'eurlex-4.3k', 'rcv1-2k']:
        raise ValueError(
            'dataset has to be amazoncat-13k, amazoncat-14k, eulrex-4.3k or rcv1-2k')
    # Download dataset
    downloader.dataset(task='extreme')

    path_to_train = None
    path_to_test = None 

    if dataset == 'amazoncat-13k':
        path_to_train = os.path.join('dataset', 'AmazonCat-13K', 'train.txt')
        path_to_test = os.path.join('dataset', 'AmazonCat-13K', 'test.txt')
    elif dataset == 'amazoncat-14k':
        path_to_train = os.path.join('dataset', 'AmazonCat-14K', 'train.txt')
        path_to_test = os.path.join('dataset', 'AmazonCat-14K', 'test.txt')
    elif dataset == 'eurlex-4.3k':
        path_to_train = os.path.join('dataset', 'EURLex-4.3K', 'train.txt')
        path_to_test = os.path.join('dataset', 'EURLex-4.3K', 'test.txt')
    elif dataset == 'rcv1-2k':
        path_to_train = os.path.join('dataset', 'RCV1-2K', 'train.txt')
        path_to_test = os.path.join('dataset', 'RCV1-2K', 'test.txt')

    assert path_to_train is not None and path_to_test is not None

    num_train, num_test, num_features, num_labels = obtain_dataset_info(
        path_to_train, path_to_test)

    ds_train = tf.data.TextLineDataset(path_to_train)
    ds_train = ds_train.skip(1).map(lambda x: tf_function(x, num_features=num_features, num_labels=num_labels)).batch(batch_size)
    ds_test = tf.data.TextLineDataset(path_to_test)
    ds_test = ds_test.skip(1).map(lambda x: tf_function(x, num_features=num_features, num_labels=num_labels)).batch(batch_size)

    return ds_train, ds_test, num_train, num_test, num_features, num_labels


def build_model(num_features, num_labels, dataset):
    """build a extreme classification model 

    Args:
        num_features (int): number of features
        num_labels (int): number of labels
        dataset (str): name of the dataset, it has to be amazoncat-13k, amazoncat-14k,
                          eurlex-4.3k or rcv1-2k 

    Return:
        (model): a keras multi-label classification model
    """

    if dataset not in ['amazoncat-13k', 'amazoncat-14k', 'eurlex-4.3k', 'rcv1-2k']:
        raise ValueError(
            'dataset has to be amazoncat-13k, amazoncat-14k, eulrex-4.3k or rcv1-2k')

    if dataset == 'amazoncat-13k':
        num_hidden_neuron = 1000
    elif dataset == 'amazoncat-14k':
        num_hidden_neuron = 1000
    elif dataset == 'eurlex-4.3k':
        num_hidden_neuron = 2000
    elif dataset == 'rcv1-2k':
        num_hidden_neuron = 5000

    model = tf.keras.Sequential([
        layers.Input(shape=(num_features)),
        layers.Dense(num_hidden_neuron, activation='relu'),
        layers.Dense(num_labels)
    ])
    optimizer = tf.keras.optimizers.Adam(learning_rate=1e-3, decay=1e-3/100)
    model.compile(optimizer=optimizer, loss=tf.keras.losses.BinaryCrossentropy(from_logits=True),
                  metrics=['accuracy', PK1_Metric])
    model.run_eagerly = True
    tf.executing_eagerly()  
    return model


def main():
    # Dataset option: amazoncat-13k, amazoncat-14k, eurlex-4.3k, rcv1-2k
    dataset = 'eurlex-4.3k'
    # Training parameters
    epochs = 5
    batch_size = 32
    # Run on CPU/GPU
    run_on_cpu = True
    run_device(run_on_cpu=run_on_cpu)

    model_name = dataset + '_xml'
    print('Training model {} with setting:'.format(model_name))
    print('dataset: {}, epochs: {}, batch_size: {}, run_on_cpu: {}'.format(
        dataset, epochs, batch_size, run_on_cpu))

    # Load dataset
    ds_train, ds_test, num_train, num_test, num_features, num_labels = load_dataset(dataset=dataset)
    ds_train = ds_train.repeat(epochs)
    print('Extreme Classification Dataset Statistics:')
    print('NUM_FEATURES: {}, NUM_LABELS: {}, NUM_TRAIN: {}, NUM_TEST: {}'.format(
        num_features, num_labels, num_train, num_test))
    # Build a model
    model = build_model(num_features, num_labels, dataset)
    # Train a model
    model.fit(ds_train, epochs=epochs, steps_per_epoch=np.floor(num_train/batch_size))
    # Evaluate the model
    model.evaluate(ds_test, steps=np.floor(num_test/batch_size))
    # Save the model
    model_save_path = model_name + '.h5'
    model.save(model_save_path)


if __name__ == "__main__":
    main()
