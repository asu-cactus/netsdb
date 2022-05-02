# do the inference via Connector-X from PostgreSQL
import connectorx as cx
import pandas
import pickle
import joblib
import psycopg2
import numpy as np
import time
import onnxruntime as rt
import hummingbird.ml
import torch

filename = 'rf-100-8-6.pkl'
input_size = 22000000
batch_size = 100000

exe_total_time = 0
total_time_start = time.time()

file_load_time_start = time.time()
loaded_model = joblib.load(filename)
file_load_time_end = time.time()

try:
    query = "SELECT leptonpT,leptoneta,leptonphi,missingenergymagnitude,missingenergyphi,"+"jet1pt,jet1eta,jet1phi,jet1btag,jet2pt,jet2eta,jet2phi,jet2btag,jet3pt,jet3eta,"+"jet3phi,jet3btag,jet4pt,jet4eta,jet4phi,jet4btag,mjj,mjjj,mlv,mjlv,mbb,mwbb,mwwbb from higgs"
    input_load_start = time.time()
    data = cx.read_sql("postgresql://postgres:postgres@localhost:5432/postgres", query)
    input_load_end = time.time()
    for i in range(int(input_size/batch_size)):
        thisdata = np.array(data[i*batch_size:(i+1)*batch_size])
        if(i == 0):
             file_trans_time_start = time.time()
             hb_onnx_model = hummingbird.ml.convert(loaded_model, "onnx", thisdata)
             file_trans_time_end = time.time()
        time_start = time.time()
        hb_onnx_model.predict(thisdata)
        time_end=time.time()
        exe_total_time = exe_total_time + (time_end-time_start)
except psycopg2.Error as e:
    t_message = "Postgres Database error: " + e + "/n"
total_time_end=time.time()

print('exe time cost',exe_total_time*1000,'ms')
print('model loading time cost',(file_load_time_end-file_load_time_start)*1000,'ms')
print('model transfer time cost',(file_trans_time_end-file_trans_time_start)*1000,'ms')
print('input loading time cost',(input_load_end-input_load_start)*1000,'ms')
print('end-to-end time cost',(total_time_end-total_time_start)*1000,'ms')