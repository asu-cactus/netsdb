import connectorx as cx
import psycopg2
import time
import os
import numpy as np
import math
from sklearn.metrics import classification_report
import treelite_runtime

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
        print(dbURL)
        print(query)
        dataframe = cx.read_sql(dbURL, query)
        print(datasetconfig["y_col"])
        if datasetconfig["type"] == "classification":
            dataframe = dataframe.astype({datasetconfig["y_col"]: int})
        end_time = time.time()
        print("Time Taken to load "+dataset+" as a dataframe is:", calulate_time(start_time,end_time))
        return dataframe
    except psycopg2.Error as e:
        print("Postgres Database error: " + e + "/n")

def get_data(data, size=-1):
    np_data = data.to_numpy() if not isinstance(data, np.ndarray) else data

    if size != -1:
        np_data = np_data[0:size, :]

    return np_data


def convert_to_hummingbird_model(model, backend, test_data, batch_size, device):
    from hummingbird.ml import constants
    from hummingbird.ml import convert, convert_batch
    remainder_size = test_data.shape[0] % batch_size 
    extra_config = {constants.N_THREADS: os.cpu_count()}
    batch_data = None
    if backend == "tvm":
        # single_batch = np.array(test_data[0:batch_size], dtype=np.float32)
        # batch_data = np.array(single_batch, dtype=np.float32)
        batch_data = np.array(test_data[0:batch_size], dtype=np.float32)
        model = convert(model, backend, batch_data, device=device, extra_config=extra_config)
    else:
        batch_data = get_data(test_data, batch_size)
        model = convert_batch(model, backend, batch_data, remainder_size, device=device, extra_config=extra_config)
    return model

def run_inference(framework,df,input_size,query_size,predict):
    start_time = time.time()
    results = []
    iterations = math.ceil(input_size/query_size)
    if framework == "TreeLite":
        def aggregate_function():
            def append(output):
                results.append(output)
            def extend(output):
                results.extend(output)
            return append if query_size == 1 else extend

        aggregate_func = aggregate_function()
        for i in range(iterations):
            query_data = treelite_runtime.DMatrix(df[i*query_size:(i+1)*query_size])
            output = predict(query_data)
            output = np.where(output > 0.5, 1, 0)
            aggregate_func(output)
    elif framework == "TFDF":
        for i in range(iterations):
            query_data = df[i*query_size:(i+1)*query_size]
            output = predict(query_data).flatten()
            output = np.where(output > 0.5, 1, 0)
            results.extend(output)
    elif framework == "HummingbirdTVMCPU":
        for i in range(iterations):
            query_data = df[i*query_size:(i+1)*query_size]
            output = predict(query_data, len(query_data)!=query_size)
            results.extend(output)
    else:
        for i in range(iterations):
            query_data = df[i*query_size:(i+1)*query_size]
            output = predict(query_data)
            results.extend(output)
    end_time = time.time()
    print("Time Taken to predict on "+framework+" is:", calulate_time(start_time,end_time))
    return results

def write_data(framework,results):
    start_time = time.time()
    # arr = np.array(results)
    # df = pd.DataFrame(arr)
    # df.to_csv(os.path.join('results','results.txt'), index=False) 
    #print(results[0:10])
    with open(os.path.join('results','results.txt'), 'w') as f:
        for item in results:
            f.write("%s\n" % item)
    
    end_time = time.time()
    print("Time Taken to write results to a text file for "+framework+" is:", calulate_time(start_time,end_time))

def find_accuracy(framework,y_actual, y_pred):
    print("Classification Report", framework)
    print(classification_report(y_actual,y_pred))
    print("################")
