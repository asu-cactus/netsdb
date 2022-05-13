from sklearn.ensemble import RandomForestClassifier
import pickle
import joblib
import psycopg2
import time
import connectorx as cx
import tensorflow_decision_forests as tfdf
import tensorflow as tf
import pandas as pd
import numpy as np

results = []

filename = 'rf-model-higgs'
model = tf.keras.models.load_model(filename)

exe_total_time = 0
total_time_start = time.time()

try:
    input_load_start = time.time()
    query = "SELECT leptonpT,leptoneta,leptonphi,missingenergymagnitude,missingenergyphi,"+"jet1pt,jet1eta,jet1phi,jet1btag,jet2pt,jet2eta,jet2phi,jet2btag,jet3pt,jet3eta,"+"jet3phi,jet3btag,jet4pt,jet4eta,jet4phi,jet4btag,mjj,mjjj,mlv,mjlv,mbb,mwbb,mwwbb from higgs"
    data = cx.read_sql("postgresql://postgres:postgres@localhost:5432/postgres", query)
    ds = tfdf.keras.pd_dataframe_to_tf_dataset(data)
    input_load_end = time.time()
    time_start = time.time()
    results.extend(model.predict(ds))
    time_end=time.time()
    exe_total_time = exe_total_time + (time_end-time_start)
except psycopg2.Error as e:
    t_message = "Postgres Database error: " + e + "/n"

writing_back_time_start = time.time()
with open('results.txt', 'w') as f:
    for item in results:
        f.write("%s\n" % item)
        
writing_back_time_end = time.time()

total_time_end=time.time()

print('End-to-End time cost',(total_time_end-total_time_start)*1000,'ms')
print('Data loading time cost',(input_load_end-input_load_start)*1000,'ms')
print('Inference time cost',exe_total_time*1000,'ms')
print('Data writing time cost',(writing_back_time_end-writing_back_time_start)*1000,'ms')