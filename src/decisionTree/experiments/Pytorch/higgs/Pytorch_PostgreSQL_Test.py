import pandas
import pickle
import joblib
import torch
import torchvision
from hummingbird.ml import convert
import psycopg2
import numpy
import time

filename = 'rf-10-8-6.pkl'
loaded_model = joblib.load(filename)
model = convert(loaded_model,'pytorch')

t_host = "localhost"
t_port = "5432"
t_dbname = "postgres"
t_user = "postgres"
t_pw = "postgres"
db_conn = psycopg2.connect(host=t_host, port=t_port, dbname=t_dbname, user=t_user, password=t_pw)
db_cursor = db_conn.cursor()

input_size = 110000
batch_size = 100
exe_total_time = 0

time_start = time.time()
try:
    for i in range(int(input_size/batch_size)):
        db_cursor.execute("SELECT leptonpT,leptoneta,leptonphi,missingenergymagnitude,missingenergyphi,"+
                          "jet1pt,jet1eta,jet1phi,jet1btag,jet2pt,jet2eta,jet2phi,jet2btag,jet3pt,jet3eta,"+
                          "jet3phi,jet3btag,jet4pt,jet4eta,jet4phi,jet4btag,mjj,mjjj,mlv,mjlv,mbb,mwbb,mwwbb from higgs;")
        some_tuple = db_cursor.fetchmany(batch_size)
        exe_time_start = time.time()
        pred = model.predict(some_tuple)
        exe_total_time = exe_total_time + time.time() - exe_time_start
except psycopg2.Error as e:
    t_message = "Postgres Database error: " + e + "/n"
time_end=time.time()

print('total time cost',(time_end-time_start)*1000,'ms')
print('exe time cost',exe_total_time*1000,'ms')
