import warnings
warnings.filterwarnings('ignore')

from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import classification_report
from xgboost import XGBClassifier
import pickle
import joblib
import numpy as np
import time
import json
import hummingbird.ml
from skl2onnx import convert_sklearn
from skl2onnx.common.data_types import FloatTensorType
import torch
import onnxruntime as rt
import os
import gc
import sys
from model_helper import *

args = sys.argv

DATASET = "higgs"
CLASSFIER = "xgboost"
gpu = False
if len(args) == 3:
    DATASET = args[1]
    CLASSFIER = args[2] 

print(DATASET)
print(CLASSFIER)

config = json.load(open("config.json"))
datasetconfig = config[DATASET]
train_size = datasetconfig["train"]
input_size = datasetconfig["rows"]
batch_size = datasetconfig["batch"]
if len(args) > 3:
    batch_size = int(args[3])
    if args[-1] == "gpu":
        gpu = True
print("Batch",batch_size)


df = fetch_data(DATASET,config)
df = df.astype({"label": int})
df_train = df.sample(frac=train_size)

num_trees = config["num_trees"]
depth = config["depth"]
y_col = datasetconfig["y_col"]
x_col = list(df_train.columns)
x_col.remove(y_col)

if not gpu:

    FRAMEWORK = "Sklearn"
    start_time = time.time()
    model = joblib.load(os.path.join("models",DATASET+"_"+CLASSFIER+"_"+str(num_trees)+"_"+str(depth)+".pkl"))
    results = run_inference(FRAMEWORK, df_train[x_col], input_size, batch_size, model.predict)
    write_data(FRAMEWORK, results)
    end_time = time.time()
    print("TOTAL Time Taken "+FRAMEWORK+" is:", calulate_time(start_time,end_time))
    find_accuracy(FRAMEWORK,df_train[y_col],results)

    del model
    del results
    gc.collect()

    # FRAMEWORK = "HummingbirdONNXCPU"
    # sklearnmodel = joblib.load(os.path.join("models",DATASET+"_"+CLASSFIER+"_"+str(num_trees)+"_"+str(depth)+".pkl"))
    # data = df[x_col]
    # model = hummingbird.ml.convert(sklearnmodel, "onnx", np.array(df[x_col]))

    # results = run_inference(FRAMEWORK, data, input_size, batch_size, model.predict)
    # write_data(FRAMEWORK, results)

    # del model
    # del sklearnmodel
    # del sess
    # del input_name
    # del label_name
    # del predict
    # del results
    # gc.collect()

    FRAMEWORK = "HummingbirdPytorchCPU"
    start_time = time.time()
    device = torch.device('cpu')
    model = torch.load(os.path.join("models",DATASET+"_"+CLASSFIER+"_"+str(num_trees)+"_"+str(depth)+"_torch.pkl"),map_location=device)
    results = run_inference(FRAMEWORK, df_train[x_col], input_size, batch_size, model.predict)
    write_data(FRAMEWORK, results)
    end_time = time.time()
    print("TOTAL Time Taken "+FRAMEWORK+" is:", calulate_time(start_time,end_time))
    find_accuracy(FRAMEWORK,df_train[y_col],results)


    del model
    del device
    del results
    gc.collect()

    FRAMEWORK = "HummingbirdTorchScriptCPU"
    start_time = time.time()
    data = df_train[x_col]
    sklearnmodel = joblib.load(os.path.join("models",DATASET+"_"+CLASSFIER+"_"+str(num_trees)+"_"+str(depth)+".pkl"))
    single_batch = np.array(data[0:batch_size], dtype=np.float32)
    torch_data = np.array(single_batch, dtype=np.float32)
    model = hummingbird.ml.convert(sklearnmodel, "torch.jit", torch_data)
    def predict(batch):
        batch = np.array(batch, dtype=np.float32)
        return model.predict(batch)

    results = run_inference(FRAMEWORK, data, input_size, batch_size, predict)
    write_data(FRAMEWORK, results)
    end_time = time.time()
    print("TOTAL Time Taken "+FRAMEWORK+" is:", calulate_time(start_time,end_time))
    find_accuracy(FRAMEWORK,df_train[y_col],results)

    del model
    del sklearnmodel
    del results
    del predict
    gc.collect()

    FRAMEWORK = "ONNXCPU"
    start_time = time.time()
    data = df_train[x_col]
    sess = rt.InferenceSession(os.path.join("models",DATASET+"_"+CLASSFIER+"_"+str(num_trees)+"_"+str(depth)+".onnx"),providers=['CPUExecutionProvider'])
    input_name = sess.get_inputs()[0].name
    label_name = sess.get_outputs()[0].name
    def predict(batch):
        output = sess.run([label_name], {input_name:np.array(batch,dtype=np.float32)})[0]
        return output

    results = run_inference(FRAMEWORK, data, input_size, batch_size, predict)
    write_data(FRAMEWORK, results)
    end_time = time.time()
    print("TOTAL Time Taken "+FRAMEWORK+" is:", calulate_time(start_time,end_time))
    find_accuracy(FRAMEWORK,df_train[y_col],results)

    del sess
    del input_name
    del label_name
    del results
    del predict
    gc.collect()
else:
    FRAMEWORK = "HummingbirdPytorchGPU"
    start_time = time.time()
    device = torch.device('cuda')
    model = torch.load(os.path.join("models",DATASET+"_"+CLASSFIER+"_"+str(num_trees)+"_"+str(depth)+"_torch.pkl"),map_location=device)
    results = run_inference(FRAMEWORK, df_train[x_col], input_size, batch_size, model.predict)
    write_data(FRAMEWORK, results)
    end_time = time.time()
    print("TOTAL Time Taken "+FRAMEWORK+" is:", calulate_time(start_time,end_time))
    find_accuracy(FRAMEWORK,df_train[y_col],results)

    del model
    del device
    del results
    gc.collect()



    FRAMEWORK = "HummingbirdTorchScriptGPU"
    start_time = time.time()
    data = df_train[x_col]
    sklearnmodel = joblib.load(os.path.join("models",DATASET+"_"+CLASSFIER+"_"+str(num_trees)+"_"+str(depth)+".pkl"))
    single_batch = np.array(data[0:batch_size], dtype=np.float32)
    torch_data = np.array(single_batch, dtype=np.float32)
    model = hummingbird.ml.convert(sklearnmodel, "torch.jit", torch_data,"cuda")
    def predict(batch):
        batch = np.array(batch, dtype=np.float32)
        return model.predict(batch)

    results = run_inference(FRAMEWORK, data, input_size, batch_size, predict)
    write_data(FRAMEWORK, results)
    end_time = time.time()
    print("TOTAL Time Taken "+FRAMEWORK+" is:", calulate_time(start_time,end_time))
    find_accuracy(FRAMEWORK,df_train[y_col],results)

    del model
    del sklearnmodel
    del results
    del predict
    gc.collect()

    FRAMEWORK = "ONNXGPU"
    start_time = time.time()
    data = df_train[x_col]
    sess = rt.InferenceSession(os.path.join("models",DATASET+"_"+CLASSFIER+"_"+str(num_trees)+"_"+str(depth)+".onnx"),providers=['CUDAExecutionProvider'])
    input_name = sess.get_inputs()[0].name
    label_name = sess.get_outputs()[0].name
    def predict(batch):
        output = sess.run([label_name], {input_name:np.array(batch,dtype=np.float32)})[0]
        return output

    results = run_inference(FRAMEWORK, data, input_size, batch_size, predict)
    write_data(FRAMEWORK, results)
    end_time = time.time()
    print("TOTAL Time Taken "+FRAMEWORK+" is:", calulate_time(start_time,end_time))
    find_accuracy(FRAMEWORK,df_train[y_col],results)

    del sess
    del input_name
    del label_name
    del results
    del predict
    gc.collect()

    FRAMEWORK = "TVM"