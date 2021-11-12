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
batch_size = 100
num_models = 6

class Word2Vec_MM():
    def __init__(self, input_weights):
        self.weights = np.copy(tf.dtypes.cast(input_weights, tf.double))
    
    def predict(self, input_batch):
        return tf.matmul(input_batch, self.weights)
    

if __name__ == "__main__":

    start = time.time()

    print("loading model")

    word2vec = tf.keras.models.load_model("word2vec")
    layer = word2vec.get_layer("w2v_embedding")
    weights= layer.get_weights()
    model_list = []
    for i in range(num_models):
        model_list.append(Word2Vec_MM(weights))

    #print ("saving model")
    #word2vec_new.save("word2vec_new", save_format="h5")

    print ("generating inputs")
    targets = tf.dtypes.cast(np.random.rand(100,vocab_size), tf.double)
    print("making inference")
    inference_start = time.time()
    for i in range(num_models):
        results = model_list[i].predict(targets)
        print(results)
    inference_end = time.time()
    print('inference time for ', num_models, ' models:', inference_end-inference_start, ' senconds')



