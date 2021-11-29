import tensorflow_hub as hub
import tensorflow as tf
import numpy as np
import pandas as pd
from tensorflow.keras import layers
import os

# create the structure of the model
model = tf.keras.Sequential()
model.add(tf.keras.Input(shape=(597540,), sparse=True))
model.add(layers.Dense(units=1000, use_bias=True, activation='relu'))
model.add(layers.Dense(units=14588, use_bias=True, activation=None))

# set the weights and bias of the model using double precision
for layer in model.layers:
    a,b = layer.get_weights()[0].shape
    w = tf.dtypes.cast(tf.random.normal([a,b], stddev = 2, mean = 0, seed =1), tf.double)
    b = tf.dtypes.cast(tf.random.normal((layer.get_weights()[1].shape), stddev = 2, mean = 0, seed =1), tf.double)
    layer.set_weights([w, b])

# compile the model
model.compile(optimizer='adam', loss='binary_crossentropy', metrics=['accuracy'])

# save the model
model.save('extreme_classification_model_double.h5')