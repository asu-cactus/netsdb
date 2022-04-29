import time
import numpy as np
from tensorflow import keras
import tensorflow as tf
from keras import layers
import psycopg2

input_dimension = 597540
batch = 1000

def get_db_connection():
    # connect to postgresql database
    t_host = "localhost"
    t_port = "5432"
    t_dbname = "postgres"
    t_user = "postgres"
    t_pw = "postgres"
    return psycopg2.connect(host=t_host, port=t_port, dbname=t_dbname, user=t_user, password=t_pw)

# Save input to DB using 
db_connection = get_db_connection()
db_cursor = db_connection.cursor()

# save the model
totalStart = time.time()
modelStart = time.time()
loaded_model = keras.models.load_model('amazon_14K.h5')
modelEnd = time.time()

# Load input from DB 
inputStart = time.time()
db_cursor.execute(f"SELECT array_data FROM images WHERE id >= {0} and id < {batch}")
rows = db_cursor.fetchall()
data = []
for row in rows:
    row = np.frombuffer(row[0], dtype=np.double)
    row = row.reshape(input_dimension)
    data.append(row)
input = np.array(data)
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