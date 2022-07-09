import warnings
warnings.filterwarnings('ignore')

from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import classification_report
from xgboost import XGBClassifier
import treelite
import treelite_runtime
import treelite.sklearn
import pickle
import joblib
import numpy as np
import time
import json
import os
import gc
import sys
from model_helper import *

args = sys.argv

DATASET = "higgs"
CLASSFIER = "xgboost"
gpu = False
print(args,len(args))
if len(args) >= 4:
    DATASET = args[1]
    CLASSFIER = args[2] 
    FRAMEWORK = args[3]
print(DATASET)
print(CLASSFIER)
print(FRAMEWORK)

config = json.load(open("config.json"))
datasetconfig = config[DATASET]
train_size = datasetconfig["train"]
input_size = datasetconfig["rows"]
batch_size = datasetconfig["batch"]
if len(args) > 4:
    batch_size = int(args[4])
    if args[-1] == "gpu":
        gpu = True
print("Batch",batch_size)


df_train = fetch_data(DATASET,config,"test")
input_size = len(df_train)
# df = df.astype({"label": int})
# df_train = df.sample(frac=train_size)

num_trees = config["num_trees"]
depth = config["depth"]
y_col = datasetconfig["y_col"]
x_col = list(df_train.columns)
x_col.remove(y_col)


if not gpu:


    if FRAMEWORK == "Sklearn":
    
        start_time = time.time()
        model = joblib.load(os.path.join("models",DATASET+"_"+CLASSFIER+"_"+str(num_trees)+"_"+str(depth)+".pkl"))
        model.set_params(verbose =0)
        model.set_params(n_jobs =-1)
        # model.set_params(verbosity =3)
        load_time = time.time()
        print("Time Taken to load sklearn model", calulate_time(start_time, load_time))
        results = run_inference(FRAMEWORK, df_train[x_col], input_size, batch_size, model.predict)
        write_data(FRAMEWORK, results)
        end_time = time.time()
        print("TOTAL Time Taken "+FRAMEWORK+" is:", calulate_time(start_time,end_time))
        find_accuracy(FRAMEWORK,df_train[y_col],results)

        del model
        del results
        gc.collect()


    elif FRAMEWORK == "TreeLite":

        start_time = time.time()
        libpath = os.path.join("models", DATASET+"_"+CLASSFIER+"_"+str(num_trees)+"_"+str(depth)+".so")
        predictor = treelite_runtime.Predictor(libpath, verbose=True)
        load_time = time.time()
        print("Time Taken to load TreeLite model", calulate_time(start_time, load_time))
        results = run_inference(FRAMEWORK, df_train[x_col], input_size, batch_size, predictor.predict)
        write_data(FRAMEWORK, results)
        end_time = time.time()
        print("TOTAL Time Taken "+FRAMEWORK+" is:", calulate_time(start_time,end_time))
        find_accuracy(FRAMEWORK,df_train[y_col],results)

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

    elif FRAMEWORK == "HummingbirdPytorchCPU":
        import hummingbird.ml
        import torch
        start_time = time.time()
        model = hummingbird.ml.load(os.path.join("models",DATASET+"_"+CLASSFIER+"_"+str(num_trees)+"_"+str(depth)+"_pytorch.pkl"))
        load_time = time.time()
        print("Time Taken to load pytorch model", calulate_time(start_time, load_time))
        #model = hummingbird.ml.convert(sklearnmodel, 'pytorch')
        #model_conversion_time = time.time()
        #model.save(os.path.join("models",DATASET+"_"+CLASSFIER+"_"+str(num_trees)+"_"+str(depth)+"_pytorch.pkl"))
        #print("Time Taken to convert HummingbirdPyTorch:",calulate_time(load_time, model_conversion_time))
        results = run_inference(FRAMEWORK, df_train[x_col], input_size, batch_size, model.predict)
        write_data(FRAMEWORK, results)
        end_time = time.time()
        print("TOTAL Time Taken "+FRAMEWORK+" is:", calulate_time(start_time,end_time))
        find_accuracy(FRAMEWORK,df_train[y_col],results)


        del model
        del results
        gc.collect()

    elif FRAMEWORK == "HummingbirdTorchScriptCPU":
        import torch
        import hummingbird.ml
        start_time = time.time()
        data = df_train[x_col]
        sklearnmodel = joblib.load(os.path.join("models",DATASET+"_"+CLASSFIER+"_"+str(num_trees)+"_"+str(depth)+".pkl"))
        sklearnmodel.set_params(verbose =0)
        sklearnmodel.set_params(n_jobs =-1)
        load_time = time.time()
        print("Time Taken to load sklearn model", calulate_time(start_time, load_time))
        single_batch = np.array(data[0:batch_size], dtype=np.float32)
        torch_data = np.array(single_batch, dtype=np.float32)
        model = hummingbird.ml.convert(sklearnmodel, "torch.jit", torch_data)
        model_conversion_time = time.time()
        print("Time Taken to convert HummingbirdTorchScript:",calulate_time(load_time, model_conversion_time))
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

    elif FRAMEWORK == "ONNXCPU":
        import onnxruntime as rt
        from skl2onnx import convert_sklearn
        from skl2onnx.common.data_types import FloatTensorType
        start_time = time.time()
        data = df_train[x_col]
        sess = rt.InferenceSession(os.path.join("models",DATASET+"_"+CLASSFIER+"_"+str(num_trees)+"_"+str(depth)+".onnx"),providers=['CPUExecutionProvider'])
        input_name = sess.get_inputs()[0].name
        label_name = sess.get_outputs()[0].name
        load_time = time.time()
        print("Time Taken to load ONNX model", calulate_time(start_time, load_time))
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
        print(FRAMEWORK + " is not supported")

else:

    if FRAMEWORK == "HummingbirdPytorchGPU":
        start_time = time.time()
        device = torch.device('cuda')
        model = torch.load(os.path.join("models",DATASET+"_"+CLASSFIER+"_"+str(num_trees)+"_"+str(depth)+"_torch.pkl"),map_location=device)
        load_time = time.time()
        print("Time Taken to load Hummingbird Pytorch GPU model", calulate_time(start_time, load_time))
        results = run_inference(FRAMEWORK, df_train[x_col], input_size, batch_size, model.predict)
        write_data(FRAMEWORK, results)
        end_time = time.time()
        print("TOTAL Time Taken "+FRAMEWORK+" is:", calulate_time(start_time,end_time))
        find_accuracy(FRAMEWORK,df_train[y_col],results)

        del model
        del device
        del results
        gc.collect()



    elif FRAMEWORK == "HummingbirdTorchScriptGPU":
        start_time = time.time()
        data = df_train[x_col]
        sklearnmodel = joblib.load(os.path.join("models",DATASET+"_"+CLASSFIER+"_"+str(num_trees)+"_"+str(depth)+".pkl"))
        sklearnmodel.set_params(n_jobs =-1)
        sklearnmodel.set_params(verbose =0)
        load_time = time.time()
        print("Time Taken to load sklearn model", calulate_time(start_time, load_time))
        single_batch = np.array(data[0:batch_size], dtype=np.float32)
        torch_data = np.array(single_batch, dtype=np.float32)
        model = hummingbird.ml.convert(sklearnmodel, "torch.jit", torch_data,"cuda")
        model_conversion_time = time.time()
        print("Time Taken to convert TorchScript GPU model", calulate_time(load_time, model_conversion_time))
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

    elif FRAMEWORK == "ONNXGPU":
        start_time = time.time()
        data = df_train[x_col]
        sess = rt.InferenceSession(os.path.join("models",DATASET+"_"+CLASSFIER+"_"+str(num_trees)+"_"+str(depth)+".onnx"),providers=['CUDAExecutionProvider'])
        input_name = sess.get_inputs()[0].name
        label_name = sess.get_outputs()[0].name
        load_time = time.time()
        print("Time Taken to load ONNX GPU model", calulate_time(start_time, load_time))
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
        print(FRAMEWORK+" is not supported")
