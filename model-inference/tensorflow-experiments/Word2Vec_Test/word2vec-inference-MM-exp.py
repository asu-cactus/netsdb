import io
import re
import string
import tqdm
import time
import sys

import numpy as np
import os
import psycopg2
import tensorflow as tf
from tensorflow.keras import layers
import tensorflow_hub as hub
from tensorflow.keras.models import Sequential
import csv

SEED = 42
AUTOTUNE = tf.data.AUTOTUNE
# define the vocabulary size and number of words in a sequence.
vocab_size = 1009375
embedding_dim = 500
batch_size = 100

class Word2Vec_MM():
    def __init__(self, input_weights):
        self.weights = np.array(tf.dtypes.cast(input_weights, tf.double))
    def predict(self, input_batch):
        return tf.matmul(input_batch, self.weights)

if __name__ == "__main__":
    # parse command-line arguments for the number of models for testing
    if(len(sys.argv)==2):
        num_models = int(sys.argv[1])
    else:
        num_models = 1
    
    # ask the user to load input from memory/CSV/Posrgres
    loading_method  = input("Enter 0 for loading from memory OR 1 for loading from Postgres one column OR 2 for loading from Postgres multiple columns OR 3 for loading from CSV file\n")
    
    # connect with the postgres
    t_host = "localhost"
    t_port = "5432"
    t_dbname = "postgres"
    t_user = "postgres"
    t_pw = "postgres"
    db_conn = psycopg2.connect(host=t_host, port=t_port, dbname=t_dbname, user=t_user, password=t_pw)
    db_cursor = db_conn.cursor()

    # load and save models based on the number of models
    print("loading model")
    word2vec = tf.keras.models.load_model(
    'w2v_wiki500_yelp_embed_nontrainable.h5', custom_objects={'KerasLayer': hub.KerasLayer})
    layer = word2vec.get_layer('keras_layer_1')
    weights= layer.get_weights()
    model_list = []
    for i in range(num_models):
        model_list.append(Word2Vec_MM(weights))
    
    # test the loading time of the inputs
    loading_start = time.time()
    if(loading_method == "0"):
        print ("loading inputs from Memory")
        targets = tf.dtypes.cast(np.random.rand(batch_size,vocab_size), tf.double)
    elif(loading_method == "1"):
        print ("loading inputs from Postgres for one column")
        targets = np.zeros([100,1009375])
        # load input from postgres
        try:
            for i in range(100):
                # execute the SELECT statement
                db_cursor.execute(""" SELECT array_data FROM text WHERE id = %s """,(i,))
                blob = db_cursor.fetchone()
                targets[i] = np.frombuffer(blob[0])
            # close the communication with the PostgresQL database
            db_cursor.close()
        except (Exception, psycopg2.DatabaseError) as error:
            print(error)
        finally:
            if db_conn is not None:
                db_conn.close()
        targets = tf.dtypes.cast(targets, tf.double)
    elif(loading_method == "2"):
        print ("loading inputs from Postgres for multiple columns")
        targets = np.zeros([100,1009375])
        try:
            for i in range(100):
                arr = None
                for j in range(400):
                    # execute the SELECT statement
                    db_cursor.execute("SELECT N" + str(j) + "Column FROM text_m WHERE id = " + str(i) + ";")
                    blob = db_cursor.fetchone()
                    this_target = np.frombuffer(blob[0])
                    if(arr is None):
                        arr = this_target
                    else:
                        arr = np.concatenate([arr,this_target],axis = 0)
                targets[i] = tf.dtypes.cast(arr, tf.double)
            # close the communication with the PostgresQL database
            db_cursor.close()
        except (Exception, psycopg2.DatabaseError) as error:
            print(error)
        finally:
            if db_conn is not None:
                db_conn.close()
    elif(loading_method == "3"):
        print ("loading inputs from CSV file")
        file = open("input_double.csv")
        targets = np.loadtxt(file, delimiter=",")
    else:
        print("Invalid Input")
    loading_end = time.time()
    print('data loading time', loading_end-loading_start, ' seconds')

    # test the inference time
    print("making inference")
    inference_start = time.time()
    for i in range(num_models):
        results = model_list[i].predict(targets)
    inference_end = time.time()
    print('inference time for ', num_models, ' models:', inference_end-inference_start, ' seconds')