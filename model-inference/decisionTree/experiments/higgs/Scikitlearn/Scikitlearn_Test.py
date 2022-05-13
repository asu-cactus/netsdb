from sklearn.ensemble import RandomForestClassifier
import pandas
import pickle
import joblib
import psycopg2
import numpy as np
import time
import connectorx as cx

filename = 'rf-10-8-6.pkl'
#file_load_time_start = time.time()
loaded_model = joblib.load(filename)
#file_load_time_end = time.time()

t_host = "localhost"
t_port = "5432"
t_dbname = "postgres"
t_user = "postgres"
t_pw = "postgres"

input_size = 22000000
batch_size = 100000
#batch_size = 22000000
results = []

exe_total_time = 0
total_time_start = time.time()

try:
    input_load_time_start = time.time()
    query = "SELECT leptonpT,leptoneta,leptonphi,missingenergymagnitude,missingenergyphi,"+"jet1pt,jet1eta,jet1phi,jet1btag,jet2pt,jet2eta,jet2phi,jet2btag,jet3pt,jet3eta,"+"jet3phi,jet3btag,jet4pt,jet4eta,jet4phi,jet4btag,mjj,mjjj,mlv,mjlv,mbb,mwbb,mwwbb from higgs"
    data = cx.read_sql("postgresql://postgres:postgres@localhost:5432/postgres", query)
    input_load_time_end = time.time()
    for i in range(int(input_size/batch_size)):
        thisdata = data[i*batch_size:(i+1)*batch_size]
        time_start = time.time()
        results.extend(loaded_model.predict(thisdata).tolist())
        time_end=time.time()
        exe_total_time = exe_total_time + (time_end-time_start)
except psycopg2.Error as e:
    t_message = "Postgres Database error: " + e + "/n"

writing_back_time_start = time.time()
"""
db_conn = psycopg2.connect(host=t_host, port=t_port, dbname=t_dbname, user=t_user, password=t_pw)
db_cursor = db_conn.cursor()
try:
    for i in range(1,22000001):
        db_cursor.execute("UPDATE higgs SET label = %s where id = %s",(float(results[i]),i))
except psycopg2.Error as e:
    t_message = "Postgres Database error: " + e + "/n"
"""

with open('results.txt', 'w') as f:
    for item in results:
        f.write("%s\n" % item)

writing_back_time_end = time.time()

total_time_end=time.time()

#print('model loading time cost',(file_load_time_end-file_load_time_start)*1000,'ms')
print('total time cost',(total_time_end-total_time_start)*1000,'ms')
print('loading input time cost',(input_load_time_end-input_load_time_start)*1000,'ms')
print('inference time cost',exe_total_time*1000,'ms')
print('writing back results time cost',(writing_back_time_end-writing_back_time_start)*1000,'ms')