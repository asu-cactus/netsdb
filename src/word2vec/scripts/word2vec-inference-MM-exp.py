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
num_models = 3 

class Word2Vec_MM():
    def __init__(self):
        self.weights = tf.dtypes.cast(np.random.rand(vocab_size, embedding_dim), tf.double)
        print(self.weights) 
    def predict(self, input_batch):
        return tf.matmul(input_batch, self.weights)
    

if __name__ == "__main__":

    print(tf.__version__)
    print(tf.config.list_physical_devices())
    print("loading model")
    model_list = []
    for i in range(num_models):
        model_list.append(Word2Vec_MM())


    print ("generating inputs")
    targets = tf.dtypes.cast(np.random.rand(batch_size,vocab_size), tf.double)
    print(targets)
    print("making inference")
    inference_start = time.time()
    for i in range(num_models):
        results = model_list[i].predict(targets)
        print(results)
    inference_end = time.time()
    print('inference time for ', num_models, ' models:', inference_end-inference_start, ' senconds')



