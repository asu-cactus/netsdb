# do the inference via Connector-X from PostgreSQL
import connectorx as cx
from sklearn.ensemble import RandomForestClassifier
import pandas
import pickle
import joblib
import psycopg2
import numpy
import time
import onnxruntime as rt

filename = "rf_higgs.onnx"
#file_load_time_start = time.time()
sess = rt.InferenceSession(filename,providers=['CPUExecutionProvider'])
#file_load_time_end = time.time()
input_name = sess.get_inputs()[0].name
label_name = sess.get_outputs()[0].name

input_size = 22000000
batch_size = 100000
results = []

exe_total_time = 0
total_time_start = time.time()

try:
    input_load_time_start = time.time()
    query = "SELECT leptonpT,leptoneta,leptonphi,missingenergymagnitude,missingenergyphi,"+"jet1pt,jet1eta,jet1phi,jet1btag,jet2pt,jet2eta,jet2phi,jet2btag,jet3pt,jet3eta,"+"jet3phi,jet3btag,jet4pt,jet4eta,jet4phi,jet4btag,mjj,mjjj,mlv,mjlv,mbb,mwbb,mwwbb from higgs"
    data = cx.read_sql("postgresql://postgres:postgres@localhost:5432/postgres", query)
    input_load_time_end = time.time()
    for i in range(int(input_size/batch_size)):
        thisdata = numpy.array(data[i*batch_size:(i+1)*batch_size])
        time_start = time.time()
        results.extend(sess.run([label_name], {input_name:thisdata.astype(numpy.float32)})[0])
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

print('total time cost',(total_time_end-total_time_start)*1000,'ms')
print('loading input time cost',(input_load_time_end-input_load_time_start)*1000,'ms')
print('inference time cost',exe_total_time*1000,'ms')
print('writing back results time cost',(writing_back_time_end-writing_back_time_start)*1000,'ms')