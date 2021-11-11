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

def create_word2vec_model(input_weights):
    word2vec = Sequential()
    embedding = layers.Embedding(vocab_size, embedding_dim, embeddings_initializer=Constant(input_weights), trainable=False)
    word2vec.add(embedding)
    return word2vec


if __name__ == "__main__":

    start = time.time()

    print("loading model")

    word2vec = tf.keras.models.load_model("word2vec")
    layer = word2vec.get_layer("w2v_embedding")
    weights= layer.get_weights()
    word2vec_new = create_word2vec_model(weights)

    #print ("saving model")
    #word2vec_new.save("word2vec_new", save_format="h5")

    print ("generating inputs")
    targets1 = np.random.randint(0, vocab_size-1, (100,))
    targets2 = np.random.randint(0, vocab_size-1, (1000,))
    targets3 = np.random.randint(0, vocab_size-1, (10000,))
    targets4 = np.random.randint(0, vocab_size-1, (100000,))
    targets5 = np.random.randint(0, vocab_size-1, (1000000,))
    #targets6 = np.random.randint(0, vocab_size-1, (10000000,))
    print("making inference")
    inference_start_1 = time.time()
    results1 = word2vec_new.predict(targets1)
    inference_start_2 = time.time()
    print('inference time for 100x1 inputs', inference_start_2-inference_start_1)
    results2 = word2vec_new.predict(targets2)
    inference_start_3 = time.time()
    print('inference time for 100x10 inputs', inference_start_3-inference_start_2)
    results3 = word2vec_new.predict(targets3)
    inference_start_4 = time.time()
    print('inference time for 100x100 inputs', inference_start_4-inference_start_3)
    results4 = word2vec_new.predict(targets4)
    inference_start_5 = time.time()
    print('inference time for 100x1000 inputs', inference_start_5-inference_start_4)
    results5 = word2vec_new.predict(targets5)
    inference_start_6 = time.time()
    print('inference time for 100x10000 inputs', inference_start_6-inference_start_5)
    #results6 = word2vec_new.predict(targets6)
    #inference_end = time.time()
    #print('inference time for 100x100000 inputs', inference_end-inference_start_6)



