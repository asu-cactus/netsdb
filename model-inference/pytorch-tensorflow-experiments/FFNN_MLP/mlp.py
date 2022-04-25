import tensorflow as tf
from tensorflow.keras import layers
import os

# create the structure of the model
model = tf.keras.Sequential()
model.add(tf.keras.Input(shape=(10000,), sparse=True))
model.add(layers.Dense(1280, activation= 'relu'))
model.add(layers.Dense(5120, activation= 'relu'))
model.add(layers.Dense(1280, activation= 'softmax'))

# set the weights and bias of the model using double precision
for layer in model.layers:
    print (layer.get_weights()[0].shape)
    a,b = layer.get_weights()[0].shape
    w = tf.dtypes.cast(tf.random.normal([a,b], stddev = 2, mean = 0, seed =1), tf.double)
    b = tf.dtypes.cast(tf.random.normal((layer.get_weights()[1].shape), stddev = 2, mean = 0, seed =1), tf.double)
    layer.set_weights([w, b])

# compile the model
model.compile(optimizer='adam', loss='binary_crossentropy', metrics=['accuracy'])

# save the model
model.save('dense_fc.h5')