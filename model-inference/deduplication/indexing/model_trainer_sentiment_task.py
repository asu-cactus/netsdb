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


def load_dataset(dataset):
    """Load dataset with given input name.
    Args:
        dataset (str): name of the dataset, it has to be civil_comment,
                          yelp_reviews, or imdb_reviews
    
    Returns:
        (array, array): return (features, labels)
    """
    if dataset not in ['civil_comment', 'yelp_reviews', 'imdb_reviews']:
        raise ValueError('dataset has to be civil_comment, yelp_reviews, or imdb_reviews')
    # Download dataset
    downloader.dataset(task='sentiment')    

    x = None
    y = None 

    df = pd.read_csv(os.path.join('dataset', dataset +'.csv'))

    x = df['text'].to_numpy()
    y = df['label'].to_numpy()

    return x, y

def build_model(trainable=True, embedding='w2v_wiki500'):
    """build a text classification model with trainable/non-trainable embedding

    Args:
        trainable (boolean): a flag to indicate the embedding layer is trainable
                             or not.
        embedding (str): name of embedding layer
    
    Return:
        (model): a keras binary classification model
    """
    # Train civil_embed_nontrainable

    if embedding == 'w2v_wiki500':
        embed_url = 'https://tfhub.dev/google/Wiki-words-500/2'
    elif embedding == 'w2v_wiki250':
        embed_url = 'https://tfhub.dev/google/Wiki-words-250/2'
    elif embedding == 'nnlm_en_dim_128':
        embed_url = 'https://tfhub.dev/google/nnlm-en-dim128/2'
    elif embedding == 'nnlm_en_dim_50':
        embed_url = 'https://tfhub.dev/google/nnlm-en-dim50/2'
    else:
        raise ValueError('Not valid embedding, valid option: w2v_wiki500, w2v_wiki250, nnln_en_dim_128, nnlm_en_dim_50')

    w2v_layer = hub.KerasLayer(embed_url, 
                input_shape=[], dtype=tf.string,trainable=trainable)
    model = tf.keras.Sequential()
    model.add(w2v_layer)
    model.add(layers.Dense(16, activation='relu'))
    model.add(layers.Dense(1, activation='sigmoid'))
    model.compile(optimizer='adam', loss='binary_crossentropy', metrics=['accuracy'])
    return model

def main():
    # Embedding option: w2v_wiki500, w2v_wiki250, nnln_en_dim_128, nnlm_en_dim_50
    embedding = 'w2v_wiki500'
    # Dataset option: civil_comment, imdb_reviews, yelp_reviews
    dataset = 'civil_comment' 
    # Trainable option: True, False
    trainable = False
    # Training parameters
    epochs = 5
    batch_size = 32
    # Run on CPU/GPU
    run_on_cpu = False

    run_device(run_on_cpu=run_on_cpu)

    model_name = embedding + '_' + dataset + '_embed_trainable' + str(trainable)
    print('Training model {} with setting:'.format(model_name))
    print('Setting: embedding: {}, trainable embedding: {}, epochs: {}, batch_size: {}, run_on_cpu: {}'.format(
                                embedding, trainable, epochs, batch_size, run_on_cpu))

    # Load dataset
    x, y = load_dataset(dataset=dataset)
    # split data into training data and testing data    
    x_train, x_test, y_train, y_test = train_test_split(x, y, 
                                        test_size=0.2, random_state=0)
    # Build a model
    model = build_model(trainable=trainable)
    # Train a model
    model.fit(x_train, y_train, batch_size=batch_size, epochs=epochs, 
            validation_data=(x_test, y_test))
    # Save a model
    model_save_path = model_name +'.h5'
    model.save(model_save_path)

if __name__ == "__main__":
    main()
