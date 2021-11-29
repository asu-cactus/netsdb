import os
import time

import tensorflow_hub as hub
import tensorflow as tf
import numpy as np
from sklearn.model_selection import train_test_split
from tensorflow.keras import layers

# load the model from h5 file
model = tf.keras.models.load_model(
    'w2v_wiki500_yelp_embed_nontrainable.h5', custom_objects={'KerasLayer': hub.KerasLayer})

# change the weights of the model
for layer in model.layers:
    try:
        if(layer.name == 'keras_layer_1'):
            layer.set_weights(tf.dtypes.cast(model.get_layer(name=layer.name).get_weights(),tf.double))
        else:
            layer.set_weights([tf.dtypes.cast(model.get_layer(name=layer.name).get_weights()[0],tf.double),tf.dtypes.cast(model.get_layer(name=layer.name).get_weights()[1],tf.double)])
    except:
        print("Could not transfer weights for layer {}".format(layer.name))

# save the new model in h5 file format with a new name
model.save('w2v_wiki500_yelp_embed_nontrainable_double.h5')