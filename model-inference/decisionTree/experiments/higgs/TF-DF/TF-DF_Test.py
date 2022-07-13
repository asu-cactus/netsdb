from sklearn.ensemble import RandomForestClassifier
import pandas
import pickle
import os
import joblib
import pathlib
import psycopg2
import numpy as np
import time
import connectorx as cx
from sklearn import tree
import tensorflow as tf
import tensorflow_decision_forests as tfdf
import scikit_learn_model_converter

DATASET = "higgs"
CLASSIFIER = "randomforest"
num_trees = 10
depth = 8
model_path = os.path.join("/home/ubuntu/netsdb/model-inference/decisionTree/experiments/models", DATASET+"_"+CLASSIFIER+"_"+str(num_trees)+"_"+str(depth)+".pkl")
#model_path = "/home/ubuntu/netsdb/model-inference/decisionTree/experiments/models/higgs_randomforest_10_8.pkl"
loaded_model = joblib.load(model_path)
tensorflow_model = scikit_learn_model_converter.convert(loaded_model,  intermediate_write_path="intermediate_path", )
libpath = os.path.join("/home/ubuntu/netsdb/model-inference/decisionTree/experiments/models", DATASET+"_"+CLASSIFIER+"_"+str(num_trees)+"_"+str(depth)+"_tfdf")
tf.saved_model.save(obj=tensorflow_model, export_dir=libpath)

#t_host = "localhost"
#t_port = "5432"
#t_dbname = "postgres"
#t_user = "postgres"
#t_pw = "postgres"

#results = []

#exe_total_time = 0
#total_time_start = time.time()

#try:
#    input_load_start = time.time()
#    query = "SELECT leptonpT,leptoneta,leptonphi,missingenergymagnitude,missingenergyphi,"+"jet1pt,jet1eta,jet1phi,jet1btag,jet2pt,jet2eta,jet2phi,jet2btag,jet3pt,jet3eta,"+"jet3phi,jet3btag,jet4pt,jet4eta,jet4phi,jet4btag,mjj,mjjj,mlv,mjlv,mbb,mwbb,mwwbb from higgs"
#    data = cx.read_sql("postgresql://postgres:postgres@localhost:5432/postgres", query)
    #ds = tfdf.keras.pd_dataframe_to_tf_dataset(data)
#    input_data = tf.constant(data)
#    input_load_end = time.time()
#    time_start = time.time()
    #results.extend(tensorflow_model.predict(ds))
#    results.extend(tensorflow_model.predict(input_data))
#    time_end=time.time()
#    exe_total_time = exe_total_time + (time_end-time_start)
#except psycopg2.Error as e:
#    t_message = "Postgres Database error: " + e + "/n"

#writing_back_time_start = time.time()
#with open('results.txt', 'w') as f:
#    for item in results:
#        f.write("%s\n" % item)
        
#writing_back_time_end = time.time()

#total_time_end=time.time()

#print('End-to-End time cost',(total_time_end-total_time_start)*1000,'ms')
#print('Data loading time cost',(input_load_end-input_load_start)*1000,'ms')
#print('Inference time cost',exe_total_time*1000,'ms')
#print('Data writing time cost',(writing_back_time_end-writing_back_time_start)*1000,'ms')
