import io
import re
import string
import tqdm
import time

import numpy as np
import os
import tensorflow as tf
from tensorflow.keras import layers
import tensorflow_hub as hub
from tensorflow.keras.models import Sequential
from keras.initializers import Constant

os.environ['CUDA_VISIBLE_DEVICES'] = '-1'

SEED = 42
AUTOTUNE = tf.data.AUTOTUNE
 # Define the vocabulary size and number of words in a sequence.
vocab_size = 1009375
embedding_dim = 500
num_models = 3 

def create_word2vec_model(input_weights):
    word2vec = Sequential()
    embedding = layers.Embedding(vocab_size, embedding_dim, embeddings_initializer=Constant(input_weights), trainable=False)
    #embedding = layers.Embedding(vocab_size, embedding_dim, embeddings_initializer=Constant(tf.dtypes.cast(input_weights, tf.double)), trainable=False)
    word2vec.add(embedding)
    return word2vec


if __name__ == "__main__":

    start = time.time()

    print("loading model")
    word2vec = tf.keras.models.load_model("/home/ubuntu/saved_model")
    word2vec = tf.keras.models.load_model("word2vec")
    layer = word2vec.get_layer("w2v_embedding")
    weights=layer.get_weights()
    model_list = []
    for i in range (num_models):
        model_list.append(create_word2vec_model(weights))
       

    print ("generating inputs")
    #targets = np.random.randint(0, vocab_size-1, (1000000,))
    targets = tf.dtypes.cast(np.random.randint(0, vocab_size-1, (1000000,)), tf.double)
    print("making inference")

    inference_start = time.time()
    for i in range(num_models):
        results = model_list[i].predict(targets)
    inference_end = time.time()
    print('inference time for all', num_models, 'models', inference_end-inference_start, ' seconds')



