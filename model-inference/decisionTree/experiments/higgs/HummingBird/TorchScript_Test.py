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

filename = 'rf-10-8-6.pkl'
loaded_model = joblib.load(filename)

input_size = 22000000
batch_size = 100000
results = []

exe_total_time = 0
total_time_start = time.time()

try:
    input_load_start = time.time()
    query = "SELECT leptonpT,leptoneta,leptonphi,missingenergymagnitude,missingenergyphi,"+"jet1pt,jet1eta,jet1phi,jet1btag,jet2pt,jet2eta,jet2phi,jet2btag,jet3pt,jet3eta,"+"jet3phi,jet3btag,jet4pt,jet4eta,jet4phi,jet4btag,mjj,mjjj,mlv,mjlv,mbb,mwbb,mwwbb from higgs"
    data = cx.read_sql("postgresql://postgres:postgres@localhost:5432/postgres", query)
    input_load_end = time.time()
    for i in range(int(input_size/batch_size)):
        thisdata = np.array(data[i*batch_size:(i+1)*batch_size], dtype=np.float32)
        if(i == 0):
            #file_trans_time_start = time.time()
            torch_data = np.array(thisdata, dtype=np.float32)
            hb_torch_model = hummingbird.ml.convert(loaded_model, "torch.jit", torch_data)
            #file_trans_time_end = time.time()
        time_start = time.time()
        results.extend(hb_torch_model.predict(thisdata))
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