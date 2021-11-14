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

class Word2Vec_MM(input_weights):
    def __init__(self):
        self.weights = np.copy(input_weights)
    
    def predict(self, input_batch):
        return tf.matmul(input_batch, self.weights)
    

if __name__ == "__main__":

    start = time.time()

    print("loading model")

    #word2vec = tf.keras.models.load_model("word2vec")
    word2vec = tf.keras.models.load_model("/home/ubuntu/saved_model")
    layer = word2vec.get_layer("w2v_embedding")
    weights= layer.get_weights()
    word2vec_new = Word2Vec_MM(weights)

    #print ("saving model")
    #word2vec_new.save("word2vec_new", save_format="h5")

    print ("generating inputs")
    targets = np.random.rand(100,vocab_size)
    print("making inference")
    inference_start_1 = time.time()
    results1 = word2vec_new.predict(targets)
    print(results1)
    inference_start_2 = time.time()
    print('inference time for 100xvocab_size input batch', inference_start_2-inference_start_1)



