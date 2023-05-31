import time
import numpy as np
from tensorflow import keras
import tensorflow as tf
from keras import layers
import connectorx as cx

input_dimension = 597540
batch = 1000

# save the model
totalStart = time.time()
modelStart = time.time()
loaded_model = keras.models.load_model('amazon_14K.h5')
modelEnd = time.time()

# Load input from DB using connectorX
inputStart = time.time()
conn_string = "postgresql://postgres:postgres@localhost:5432/postgres"
query = f"SELECT * FROM images where id >= {0} and id < {1000}"
data = cx.read_sql(conn_string, query)
df = data["array_data"]

input = []
for bytes in df:
    currentBytes = np.frombuffer(bytes, dtype=np.double)
    currentBytes = currentBytes.reshape(input_dimension)
    input.append(currentBytes)
input = np.array(input)
inputEnd = time.time()
totalEnd = time.time()


print ("Input shape", input.shape)
startTime = time.time()
output = loaded_model.predict(input)
endTime = time.time()
print("Output Shape", output.shape)
totalEnd = time.time()

print ("Total Time:", totalEnd - totalStart)
print ("Sum Total:", (inputEnd - inputStart) + (modelEnd - modelStart) + (endTime - startTime))
print ("Input Load Time: ", inputEnd - inputStart)
print ("Model Load Time:", modelEnd - modelStart)
print ("Inference Time:", endTime - startTime)