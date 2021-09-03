import tensorflow as tf
import numpy as np
import tensorflow_hub as hub

model_dir = '/home/ubuntu/word2vec-finetune'

hub_layer = hub.KerasLayer("https://tfhub.dev/google/Wiki-words-500/2", input_shape=[], dtype=tf.string)
layer1_weights = hub_layer.get_weights()

model2 = tf.keras.models.load_model(model_dir)
layer2 = model2.get_layer("w2v_embedding")
layer2_weights = layer2.get_weights()

model_diff = []
layer_diff = layer1_weights[0] - layer2_weights[0]
model_diff.append(layer_diff)
print(layer_diff)

for i in range(len(model_diff)):
  current_layer_size = 0
  total_nonzero = 0
  max = 0
  for cell in np.nditer(model_diff[i]):
    current_layer_size += 1
    if abs(cell) > 0.001:
      total_nonzero += 1
      if abs(cell) > max:
        max = cell
  percentage_diff = ((total_nonzero * 1.) / current_layer_size) * 100
  print("Amount of different weights in Layer " + str(i + 1) + ": " + str(total_nonzero)
        + " / " + str(current_layer_size) + " (" + str(percentage_diff) + "%)")
  print("Maximum Difference in Layer " + str(i+1) + ": " + str(max))

