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

if __name__ == "__main__":

    # Define the vocabulary size and number of words in a sequence.
    vocab_size = 1009375
    embedding_dim = 500

    start = time.time()

    print("loading model-1")

    targets1 = np.random.randint(0, vocab_size-1, (100,))
    #print(targets1)
    #print(targets1.shape)

    #dataset = tf.data.Dataset.from_tensor_slices(((targets, contexts), labels))
    #dataset = dataset.shuffle(BUFFER_SIZE).batch(BATCH_SIZE, drop_remainder=True)
    #dataset = dataset.cache().prefetch(buffer_size=AUTOTUNE)
    #print(dataset)

    word2vec = tf.keras.models.load_model("word2vec")
    layer2 = word2vec.get_layer("w2v_embedding")
    weights1= layer2.get_weights()
    word2vec1 = Sequential()
    embedding1 = layers.Embedding(vocab_size, embedding_dim, embeddings_initializer=Constant(weights1), trainable=False)
    word2vec1.add(embedding1)

    print("loading model-2")

    targets2 = np.random.randint(0, vocab_size-1, (100,))
    weights2 = layer2.get_weights()
    word2vec2 = Sequential()
    embedding2 = layers.Embedding(vocab_size, embedding_dim, embeddings_initializer=Constant(weights2), trainable=False)
    word2vec2.add(embedding2)


    print("loading model-3")

    targets3 = np.random.randint(0, vocab_size-1, (100,))
    weights3 = layer2.get_weights()
    word2vec3 = Sequential()
    embedding3 = layers.Embedding(vocab_size, embedding_dim, embeddings_initializer=Constant(weights3), trainable=False)
    word2vec3.add(embedding3)

    print("loading model-4")

    targets4 = np.random.randint(0, vocab_size-1, (100,))
    weights4 = layer2.get_weights()
    word2vec4 = Sequential()
    embedding4 = layers.Embedding(vocab_size, embedding_dim, embeddings_initializer=Constant(weights4), trainable=False)
    word2vec4.add(embedding4)

    print("loading model-5")

    targets5 = np.random.randint(0, vocab_size-1, (100,))
    weights5 = layer2.get_weights()
    word2vec5 = Sequential()
    embedding5 = layers.Embedding(vocab_size, embedding_dim, embeddings_initializer=Constant(weights5), trainable=False)
    word2vec5.add(embedding5)


#    print("loading model-6")

#    targets6 = np.random.randint(0, vocab_size-1, (100,))
#    weights6 = layer2.get_weights()
#    word2vec6 = Sequential()
#    embedding6 = layers.Embedding(vocab_size, embedding_dim, embeddings_initializer=Constant(weights6), trainable=False)
#    word2vec6.add(embedding5)


#    print("loading model-7")

#    targets7 = np.random.randint(0, vocab_size-1, (100,))
#    weights7 = layer2.get_weights()
#    word2vec7 = Sequential()
#    embedding7 = layers.Embedding(vocab_size, embedding_dim, embeddings_initializer=Constant(weights7), trainable=False)
#    word2vec7.add(embedding2)


#    print("loading model-8")

#    targets8 = np.random.randint(0, vocab_size-1, (100,))
#    weights8 = layer2.get_weights()
#    word2vec8 = Sequential()
#    embedding8 = layers.Embedding(vocab_size, embedding_dim, embeddings_initializer=Constant(weights8), trainable=False)
#    word2vec8.add(embedding3)

#    print("loading model-9")

#    targets9 = np.random.randint(0, vocab_size-1, (100,))
#    weights9 = layer2.get_weights()
#    word2vec9 = Sequential()
#    embedding9 = layers.Embedding(vocab_size, embedding_dim, embeddings_initializer=Constant(weights9), trainable=False)
#    word2vec9.add(embedding4)

#    print("loading model-10")

#    targets10 = np.random.randint(0, vocab_size-1, (100,))
#    weights10 = layer2.get_weights()
#    word2vec10 = Sequential()
#    embedding10 = layers.Embedding(vocab_size, embedding_dim, embeddings_initializer=Constant(weights10), trainable=False)
#    word2vec10.add(embedding5)




    inference_start = time.time()
    results1 = word2vec1.predict(targets1)
    results2 = word2vec2.predict(targets2)
    results3 = word2vec3.predict(targets3)
    results4 = word2vec4.predict(targets4)
    results5 = word2vec5.predict(targets5)
#    results6 = word2vec6.predict(targets6)
#    results7 = word2vec7.predict(targets7)
#    results8 = word2vec8.predict(targets8)
#    results9 = word2vec9.predict(targets9)
#    results10 = word2vec10.predict(targets10)
    inference_end = time.time()
    print(results1)
    print(results2)
    print(results3)
    print(results4)
    print(results5)
#    print(results6)
#    print(results7)
#    print(results8)
#    print(results9)
#    print(results10)
    print('model and data preparation time', inference_start-start)
    print('inference time for word2vec', inference_end-inference_start)
