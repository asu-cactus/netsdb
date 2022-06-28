import connectorx as cx
import psycopg2
import json
import time
import os
import numpy as np
import pandas as pd
import math
from sklearn.metrics import classification_report

def calulate_time(start_time,end_time):
    diff = (end_time-start_time)*1000
    return diff

def fetch_data(dataset,config,suffix):
    try:
        pgsqlconfig = config["pgsqlconfig"]
        datasetconfig = config[dataset]
        query = datasetconfig["query"]+"_"+suffix
        dbURL = "postgresql://"+pgsqlconfig["username"]+":"+pgsqlconfig["password"]+"@"+pgsqlconfig["host"]+":"+pgsqlconfig["port"]+"/"+pgsqlconfig["dbname"]
        start_time = time.time()
        dataframe = cx.read_sql(dbURL, query)
        dataframe = dataframe.astype({"label": int})
        end_time = time.time()
        print("Time Taken to load "+dataset+" as a dataframe is:", calulate_time(start_time,end_time))
        return dataframe
    except psycopg2.Error as e:
        print("Postgres Database error: " + e + "/n")

def run_inference(framework,df,input_size,batch_size,predict):
    start_time = time.time()
    results = []
    iterations = math.ceil(input_size/batch_size)
    for i in range(iterations):
        batch = df[i*batch_size:(i+1)*batch_size]
        output = predict(batch)
        results.extend(output)
    end_time = time.time()
    print("Time Taken to predict on "+framework+" is:", calulate_time(start_time,end_time))
    return results

def write_data(framework,results):
    start_time = time.time()
    # arr = np.array(results)
    # df = pd.DataFrame(arr)
    # df.to_csv(os.path.join('results','results.txt'), index=False) 
    with open(os.path.join('results','results.txt'), 'w') as f:
        for item in results:
            f.write("%s\n" % item)
    
    end_time = time.time()
    print("Time Taken to write results to a text file for "+framework+" is:", calulate_time(start_time,end_time))

def find_accuracy(framework,y_actual, y_pred):
    print("Classification Report", framework)
    print(classification_report(y_actual,y_pred))
    print("################")
