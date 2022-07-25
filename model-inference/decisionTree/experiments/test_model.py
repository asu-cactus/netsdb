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

if len(args) < 4:
    print("Usage: python3 ./test_model.py DATASET CLASSIFIER FRAMEWORK QUERY_SIZE BATCH_SIZE [gpu/cpu]")
    print("For HummingbirdTVM, QUERY_SIZE must be equivalent to BATCH_SIZE.")
    print("For Sklearn, TreeLite, ONNX, only QUERY_SIZE is used to split the inference into multiple queries, and BATCH_SIZE will not be used.")
    print("For other platforms, both QUERY_SIZE and BATCH_SIZE will be used.")

DATASET = "higgs"
CLASSIFIER = "xgboost"
gpu = False
print(args,len(args))
if len(args) >= 4:
    DATASET = args[1]
    CLASSIFIER = args[2] 
    FRAMEWORK = args[3]
print(DATASET)
print(CLASSIFIER)
print(FRAMEWORK)

config = json.load(open("config.json"))
datasetconfig = config[DATASET]
train_size = datasetconfig["train"]
input_size = datasetconfig["rows"]
query_size = datasetconfig["query_size"]
batch_size = datasetconfig["batch_size"]

if len(args) > 4:
    query_size = int(args[4])

if len(args) > 5:
    batch_size = int(args[5])
    
if args[-1] == "gpu":
        gpu = True

print("Query Size:", query_size)
print("Batch Size:", batch_size)

df_test = fetch_data(DATASET,config,"test")
input_size = len(df_test)

num_trees = config["num_trees"]
depth = config["depth"]
y_col = datasetconfig["y_col"]
x_col = list(df_test.columns)
x_col.remove(y_col)

start_time = time.time()
sklearnmodel = joblib.load(os.path.join("models",DATASET+"_"+CLASSIFIER+"_"+str(num_trees)+"_"+str(depth)+".pkl"))
sklearnmodel.set_params(verbose =0)
sklearnmodel.set_params(n_jobs =-1)
load_time = time.time()
print("Time Taken to load sklearn model", calulate_time(start_time, load_time))

if not gpu:

    if FRAMEWORK == "Sklearn":
        start_time = time.time()
        #scikit-learn will use all data in a query as one batch  
        results = run_inference(FRAMEWORK, df_test[x_col], input_size, query_size, sklearnmodel.predict)
        write_data(FRAMEWORK, results)
        end_time = time.time()
        print("TOTAL Time Taken "+FRAMEWORK+" is:", calulate_time(start_time,end_time))
        find_accuracy(FRAMEWORK,df_test[y_col],results)

        del results
        gc.collect()


    elif FRAMEWORK == "TreeLite":
        start_time = time.time()
        libpath = os.path.join("models", DATASET+"_"+CLASSIFIER+"_"+str(num_trees)+"_"+str(depth)+".so")
        predictor = treelite_runtime.Predictor(libpath, verbose=True)
        load_time = time.time()
        print("Time Taken to load TreeLite model", calulate_time(start_time, load_time))
        results = run_inference(FRAMEWORK, df_test[x_col], input_size, query_size, predictor.predict)
        write_data(FRAMEWORK, results)
        end_time = time.time()
        print("TOTAL Time Taken "+FRAMEWORK+" is:", calulate_time(start_time,end_time))
        find_accuracy(FRAMEWORK,df_test[y_col],results)

        del results
        gc.collect()

    #https://github.com/microsoft/hummingbird/blob/main/hummingbird/ml/convert.py#L447
    elif FRAMEWORK == "HummingbirdPytorchCPU":
        import hummingbird.ml
        import torch
        start_time = time.time()
        model = convert_to_hummingbird_model(sklearnmodel, "torch", df_test[x_col], batch_size, "cpu")
        model_conversion_time = time.time()
        print("Time Taken to convert HummingbirdPyTorch:",calulate_time(start_time, model_conversion_time))

        results = run_inference(FRAMEWORK, df_test[x_col], input_size, query_size, model.predict)
        write_data(FRAMEWORK, results)
        end_time = time.time()
        print("TOTAL Time Taken "+FRAMEWORK+" is:", calulate_time(start_time,end_time))
        find_accuracy(FRAMEWORK,df_test[y_col],results)


        del model
        del results
        gc.collect()

    elif FRAMEWORK == "HummingbirdTorchScriptCPU":
        import torch
        import hummingbird.ml
        start_time = time.time()
        data = df_test[x_col]
        model = convert_to_hummingbird_model(sklearnmodel, "torch.jit", data, batch_size, "cpu")
        model_conversion_time = time.time()
        print("Time Taken to convert HummingbirdTorchScript:",calulate_time(start_time, model_conversion_time))
        def predict(batch):
            batch = np.array(batch, dtype=np.float32)
            return model.predict(batch)

        results = run_inference(FRAMEWORK, data, input_size, query_size, predict)
        write_data(FRAMEWORK, results)
        end_time = time.time()
        print("TOTAL Time Taken "+FRAMEWORK+" is:", calulate_time(start_time,end_time))
        find_accuracy(FRAMEWORK,df_test[y_col],results)

        del model
        del results
        del predict
        gc.collect()


    elif FRAMEWORK == "HummingbirdTVMCPU":
        import hummingbird.ml
        assert batch_size == query_size, "For TVM, batch_size must be equivalent to query_size"
        start_time = time.time()
        data = df_test[x_col]
        model = convert_to_hummingbird_model(sklearnmodel, "tvm", data, batch_size, "cpu")
        model_conversion_time = time.time()
        print("Time Taken to convert HummingbirdTVM:",calulate_time(load_time, model_conversion_time))
        def predict(batch):
            batch = np.array(batch, dtype=np.float32)
            return model.predict(batch)

        results = run_inference(FRAMEWORK, data, input_size, query_size, predict)
        write_data(FRAMEWORK, results)
        end_time = time.time()
        print("TOTAL Time Taken "+FRAMEWORK+" is:", calulate_time(start_time,end_time))
        find_accuracy(FRAMEWORK,df_test[y_col],results)

        del model
        del results
        del predict
        gc.collect()

    elif FRAMEWORK == "TFDF":
        import tensorflow as tf
        import tensorflow_decision_forests as tfdf
        import scikit_learn_model_converter
        import xgboost_model_converter
        start_time = time.time()
        data = df_test[x_col]
        model = None
        if CLASSIFIER == "randomforest":
            model = scikit_learn_model_converter.convert(sklearnmodel, intermediate_write_path="intermediate_path", )
        else:
            model = xgboost_model_converter.convert(sklearnmodel, intermediate_write_path="intermediate_path", )
        model_conversion_time = time.time()
        print("Time Taken to convert TensorFlow::",calulate_time(load_time, model_conversion_time))

        def predict(batch):
            batch = tf.constant(batch)
            return model.predict(batch, batch_size = batch_size)

        results = run_inference(FRAMEWORK, data, input_size, query_size, predict)
        write_data(FRAMEWORK, results)
        end_time = time.time()
        print("TOTAL Time Taken "+FRAMEWORK+" is:", calulate_time(start_time,end_time))
        find_accuracy(FRAMEWORK,df_test[y_col],results)

        del model
        del results
        del predict
        gc.collect()

    elif FRAMEWORK == "ONNXCPU":
        import onnxruntime as rt
        from skl2onnx import convert_sklearn
        from skl2onnx.common.data_types import FloatTensorType
        #https://github.com/microsoft/onnxruntime-openenclave/blob/openenclave-public/docs/ONNX_Runtime_Perf_Tuning.md
        sess_opt = rt.SessionOptions()
        sess_opt.intra_op_num_threads = 2
        sess_opt.execution_mode = rt.ExecutionMode.ORT_SEQUENTIAL
        start_time = time.time()
        data = df_test[x_col]
        sess = rt.InferenceSession(os.path.join("models",DATASET+"_"+CLASSIFIER+"_"+str(num_trees)+"_"+str(depth)+".onnx"),providers=['CPUExecutionProvider'], sess_options=sess_opt)
        input_name = sess.get_inputs()[0].name
        label_name = sess.get_outputs()[0].name
        load_time = time.time()
        print("Time Taken to load ONNX model", calulate_time(start_time, load_time))
        def predict(batch):
            output = sess.run([label_name], {input_name:np.array(batch,dtype=np.float32)})[0]
            return output

        results = run_inference(FRAMEWORK, data, input_size, query_size, predict)
        write_data(FRAMEWORK, results)
        end_time = time.time()
        print("TOTAL Time Taken "+FRAMEWORK+" is:", calulate_time(start_time,end_time))
        find_accuracy(FRAMEWORK,df_test[y_col],results)

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
        model = torch.load(os.path.join("models",DATASET+"_"+CLASSIFIER+"_"+str(num_trees)+"_"+str(depth)+"_torch.pkl"),map_location=device)
        load_time = time.time()
        print("Time Taken to load Hummingbird Pytorch GPU model", calulate_time(start_time, load_time))
        results = run_inference(FRAMEWORK, df_test[x_col], input_size, query_size, model.predict)
        write_data(FRAMEWORK, results)
        end_time = time.time()
        print("TOTAL Time Taken "+FRAMEWORK+" is:", calulate_time(start_time,end_time))
        find_accuracy(FRAMEWORK,df_test[y_col],results)

        del model
        del device
        del results
        gc.collect()



    elif FRAMEWORK == "HummingbirdTorchScriptGPU":
        start_time = time.time()
        data = df_test[x_col]
        single_batch = np.array(data[0:query_size], dtype=np.float32)
        torch_data = np.array(single_batch, dtype=np.float32)
        model = hummingbird.ml.convert(sklearnmodel, "torch.jit", torch_data,"cuda")
        model_conversion_time = time.time()
        print("Time Taken to convert TorchScript GPU model", calulate_time(start_time, model_conversion_time))
        def predict(batch):
            batch = np.array(batch, dtype=np.float32)
            return model.predict(batch)

        results = run_inference(FRAMEWORK, data, input_size, query_size, predict)
        write_data(FRAMEWORK, results)
        end_time = time.time()
        print("TOTAL Time Taken "+FRAMEWORK+" is:", calulate_time(start_time,end_time))
        find_accuracy(FRAMEWORK,df_test[y_col],results)

        del model
        del sklearnmodel
        del results
        del predict
        gc.collect()

    elif FRAMEWORK == "ONNXGPU":
        start_time = time.time()
        data = df_test[x_col]
        sess = rt.InferenceSession(os.path.join("models",DATASET+"_"+CLASSIFIER+"_"+str(num_trees)+"_"+str(depth)+".onnx"),providers=['CUDAExecutionProvider'])
        input_name = sess.get_inputs()[0].name
        label_name = sess.get_outputs()[0].name
        load_time = time.time()
        print("Time Taken to load ONNX GPU model", calulate_time(start_time, load_time))
        def predict(batch):
            output = sess.run([label_name], {input_name:np.array(batch,dtype=np.float32)})[0]
            return output

        results = run_inference(FRAMEWORK, data, input_size, query_size, predict)
        write_data(FRAMEWORK, results)
        end_time = time.time()
        print("TOTAL Time Taken "+FRAMEWORK+" is:", calulate_time(start_time,end_time))
        find_accuracy(FRAMEWORK,df_test[y_col],results)

        del sess
        del input_name
        del label_name
        del results
        del predict
        gc.collect()

    else:
        print(FRAMEWORK+" is not supported")


del sklearnmodel
