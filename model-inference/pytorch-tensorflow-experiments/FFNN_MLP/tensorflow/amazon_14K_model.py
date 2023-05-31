import time
import psycopg2
import numpy as np
from tensorflow import keras
import tensorflow as tf
from keras import layers

input_dimension = 597540
input_path = 'amazon_14K_Input'
batch = 1000

# create the structure of the model
model = tf.keras.Sequential()
model.add(tf.keras.Input(shape=(input_dimension,), batch_size=batch))
model.add(layers.Dense(1024, activation= 'relu'))
model.add(layers.Dense(14588, activation= 'softmax'))

# set the weights and bias of the model using double precision
for layer in model.layers:
    print (layer.get_weights()[0].shape)
    a,b = layer.get_weights()[0].shape
    w = tf.dtypes.cast(tf.random.normal([a,b], stddev = 2, mean = 0, seed =1), tf.double)
    b = tf.dtypes.cast(tf.random.normal((layer.get_weights()[1].shape), stddev = 2, mean = 0, seed =1), tf.double)
    layer.set_weights([w, b])

# compile the model
model.compile()

# # save the model
model.save('amazon_14K.h5')

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

db_cursor.execute("DROP TABLE IF EXISTS images;")
db_cursor.execute("CREATE TABLE IF NOT EXISTS images(id int, array_data bytea);")

for id in range(0, batch):
    img = np.random.rand(input_dimension)
    img_bytes = img.tobytes()
    db_cursor.execute("INSERT INTO images(id,array_data) " + "VALUES(%s,%s)",(id, img_bytes))

# commit the changes to the database
db_connection.commit()
db_cursor.close()