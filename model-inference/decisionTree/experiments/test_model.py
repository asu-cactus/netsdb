from pickle import FRAME
import warnings
warnings.filterwarnings('ignore')

import treelite_runtime
import joblib
import numpy as np
import time
import json
import os
import csv
import argparse
from model_helper import *

# Default arguments
DATASET = "higgs"
MODEL = "xgboost"
FRAMEWORK = "Sklearn"


def parse_arguments(config):
    global DATASET, MODEL, FRAMEWORK
    parser = argparse.ArgumentParser(description="""
        Parse arguments for test_model.py
        Usage: python3 ./test_model.py DATASET MODEL FRAMEWORK QUERY_SIZE BATCH_SIZE [gpu/cpu]
        For Sklearn, TreeLite, ONNX, only QUERY_SIZE is used to split the inference into multiple queries, and BATCH_SIZE will not be used.
        For other platforms, both QUERY_SIZE and BATCH_SIZE will be used.
    """)
    parser.add_argument(
        "-d", "--dataset", type=str, 
        choices=['higgs', 'airline_regression', 'airline_classification', 'fraud', 'year', 'epsilon', 'bosch', 'covtype'],
        help="Dataset to be tested.")
    parser.add_argument(
        "-m", "--model", type=str,  
        choices=['randomforest', 'xgboost', 'lightgbm'],
        help="Model name. Choose from ['randomforest', 'xgboost', 'lightgbm']")
    parser.add_argument(
        "-f", "--frameworks", type=str,
        choices=[
            'Sklearn', 
            'TreeLite', 
            'HummingbirdPytorchCPU', 
            'HummingbirdTorchScriptCPU', 
            'HummingbirdTVMCPU', 
            'TFDF',
            'ONNXCPU', 
            'LightGBM',
            'Lleaves',
            'HummingbirdPytorchGPU',
            'HummingbirdTorchScriptGPU',
            'ONNXGPU',
            'HummingbirdTVMGPU',
            'NvidiaFILGPU',
            'XGBoostGPU'
        ],
        help="Framework to run the decision forest model.")
    parser.add_argument("--batch_size", type=int, 
        help="Batch size for testing. For Sklearn, TreeLite, ONNX, batch_size will not be used.")
    parser.add_argument("--query_size", type=int, help="Query size for testing.")
    args = parser.parse_args()
    check_argument_conflicts(args)
    if args.dataset:
        DATASET = args.dataset
    if args.model:
        MODEL = args.model
    if args.frameworks:
        FRAMEWORK = args.frameworks
    if not args.batch_size:
        args.batch_size = config[DATASET]["batch_size"]
    if not args.query_size:
        args.query_size = config[DATASET]["query_size"]

    if DATASET == "year" or DATASET == "airline_regression":
        args.task_type = "regression"
    else:
        args.task_type = "classification"
    # Print arguments
    print(f"DATASET: {DATASET}")
    print(f"MODEL: {MODEL}")
    print(f"FRAMEWORK: {FRAMEWORK}")
    print(f"Query Size: {args.query_size}")
    print(f"Batch Size: {args.batch_size}")
    return args


def load_data(config, time_consume):
    df_test = fetch_data(DATASET, config, "test", time_consume=time_consume)
    y_col = config[DATASET]["y_col"]
    x_col = list(df_test.columns)
    x_col.remove(y_col)
    features = df_test[x_col].to_numpy(dtype=np.float32)
    label = df_test[y_col]
    return (features, label)

def load_sklearn_model(config, time_consume):
    start_time = time.time()
    relative_path = relative2abspath("models", f"{DATASET}_{MODEL}_{config['num_trees']}_{config['depth']}.pkl")
    sklearnmodel = joblib.load(relative_path)
    sklearnmodel.set_params(verbose =0)
    sklearnmodel.set_params(n_jobs =-1)
    load_time = time.time()
    sklearnmodel_loading_time = calculate_time(start_time, load_time)
    time_consume["sklearn loading time"] = sklearnmodel_loading_time
    print(f"Time Taken to load sklearn model: {sklearnmodel_loading_time}")
    return sklearnmodel


def test(*argv):
    if FRAMEWORK.endswith("GPU"):
        test_postprocess(*test_gpu(*argv))
    else:
        test_postprocess(*test_cpu(*argv))

def test_cpu(args, features, label, sklearnmodel, config, time_consume):
    input_size = len(label)

    if FRAMEWORK == "Sklearn":
        start_time = time.time()
        #scikit-learn will use all data in a query as one batch  
        conversion_time = 0.0
        results = run_inference(FRAMEWORK, features, input_size, args.query_size, sklearnmodel.predict, time_consume)
        write_data(FRAMEWORK, results, time_consume) 
        total_framework_time = calculate_time(start_time, time.time())

    elif FRAMEWORK == "TreeLite":
        start_time = time.time()
        libpath = relative2abspath("models", f"{DATASET}_{MODEL}_{config['num_trees']}_{config['depth']}.so")
        predictor = treelite_runtime.Predictor(libpath, verbose=True)
        conversion_time = calculate_time(start_time, time.time())
        results = run_inference(FRAMEWORK, features, input_size, args.query_size, predictor.predict, time_consume)
        write_data(FRAMEWORK, results, time_consume)
        total_framework_time = calculate_time(start_time, time.time())

    #https://github.com/microsoft/hummingbird/blob/main/hummingbird/ml/convert.py#L447
    elif FRAMEWORK == "HummingbirdPytorchCPU":
        start_time = time.time()
        model = convert_to_hummingbird_model(sklearnmodel, "torch", features, args.batch_size, "cpu")
        conversion_time = calculate_time(start_time, time.time())
        results = run_inference(FRAMEWORK, features, input_size, args.query_size, model.predict, time_consume)
        write_data(FRAMEWORK, results, time_consume)
        total_framework_time = calculate_time(start_time, time.time())

    elif FRAMEWORK == "HummingbirdTorchScriptCPU":
        start_time = time.time()
        model = convert_to_hummingbird_model(sklearnmodel, "torch.jit", features, args.batch_size, "cpu")
        conversion_time = calculate_time(start_time, time.time())
        def predict(batch):
            return model.predict(batch)

        results = run_inference(FRAMEWORK, features, input_size, args.query_size, predict, time_consume)
        write_data(FRAMEWORK, results, time_consume)
        total_framework_time = calculate_time(start_time, time.time())

    elif FRAMEWORK == "HummingbirdTVMCPU":
        assert args.batch_size == args.query_size, "For TVM, batch_size must be equivalent to query_size"
        start_time = time.time()
        model = convert_to_hummingbird_model(sklearnmodel, "tvm", features, args.batch_size, "cpu")
        remainder_size = input_size % args.batch_size
        if remainder_size > 0:
            remainder_model = convert_to_hummingbird_model(sklearnmodel, "tvm", features, remainder_size, "cpu")
        conversion_time = calculate_time(start_time, time.time())
        def predict(batch, use_remainder_model):
            if use_remainder_model:
                return remainder_model.predict(batch)
            return model.predict(batch)

        results = run_inference(FRAMEWORK, features, input_size, args.query_size, predict, time_consume)
        write_data(FRAMEWORK, results, time_consume)
        total_framework_time = calculate_time(start_time, time.time())

    elif FRAMEWORK == "TFDF":
        import tensorflow as tf
        import tensorflow_decision_forests as tfdf
        import scikit_learn_model_converter
        import xgboost_model_converter
        start_time = time.time()
        if MODEL == "randomforest":
            model = scikit_learn_model_converter.convert(sklearnmodel, intermediate_write_path="intermediate_path", )
        else:
            model = xgboost_model_converter.convert(sklearnmodel, intermediate_write_path="intermediate_path", )
        conversion_time = calculate_time(start_time, time.time())
        def predict(batch):
            batch = tf.constant(batch)
            return model.predict(batch, batch_size=args.batch_size)

        results = run_inference(FRAMEWORK, features, input_size, args.query_size, predict, time_consume)
        write_data(FRAMEWORK, results, time_consume)
        total_framework_time = calculate_time(start_time, time.time())

    elif FRAMEWORK == "ONNXCPU":
        import onnxruntime as rt
        #https://github.com/microsoft/onnxruntime-openenclave/blob/openenclave-public/docs/ONNX_Runtime_Perf_Tuning.md
        sess_opt = rt.SessionOptions()
        sess_opt.intra_op_num_threads = os.cpu_count() 
        sess_opt.execution_mode = rt.ExecutionMode.ORT_SEQUENTIAL
        start_time = time.time()
        relative_path = relative2abspath("models", f"{DATASET}_{MODEL}_{config['num_trees']}_{config['depth']}.onnx")
        sess = rt.InferenceSession(relative_path,providers=['CPUExecutionProvider'], sess_options=sess_opt)
        input_name = sess.get_inputs()[0].name
        label_name = sess.get_outputs()[0].name
        conversion_time = calculate_time(start_time, time.time())
        def predict(batch):
            output = sess.run([label_name], {input_name:batch})[0]
            return output

        results = run_inference(FRAMEWORK, features, input_size, args.query_size, predict, time_consume)
        write_data(FRAMEWORK, results, time_consume)
        total_framework_time = calculate_time(start_time, time.time())
    elif FRAMEWORK == "LightGBM":
        import lightgbm
        # LightGBM Model Conversion & Inference
        start_time = time.time()
        model_path = relative2abspath("models", f"{DATASET}_{MODEL}_{config['num_trees']}_{config['depth']}.txt")
        model = lightgbm.Booster(model_file=model_path)
        conversion_time = calculate_time(start_time, time.time())
        def predict(batch):
            return model.predict(batch)
        results = run_inference(FRAMEWORK, features, input_size, args.query_size, predict, time_consume)
        write_data(FRAMEWORK, results, time_consume)
        total_framework_time = calculate_time(start_time, time.time())
    elif FRAMEWORK == "Lleaves":
        import lleaves
        # Lleaves Model Conversion & Inference
        start_time = time.time()
        model_path = relative2abspath("models", f"{DATASET}_{MODEL}_{config['num_trees']}_{config['depth']}.txt")
        model = lleaves.Model(model_file=model_path)
        model_cache_path = relative2abspath("models", f"{DATASET}_{MODEL}_{config['num_trees']}_{config['depth']}.elf")
        model.compile(cache=model_cache_path)  # NOTE: Using Cache because of the extremely long compilation times for 500, 1600 Trees Models.
        conversion_time = calculate_time(start_time, time.time())
        def predict(batch):
            return model.predict(batch)
        results = run_inference(FRAMEWORK, features, input_size, args.query_size, predict, time_consume)
        write_data(FRAMEWORK, results, time_consume)
        total_framework_time = calculate_time(start_time, time.time())
    else:
        raise ValueError(f"{FRAMEWORK} is not supported.")
    if args.task_type == "classification":
        find_accuracy(FRAMEWORK, label, results)
    else:
        find_MSE(FRAMEWORK, label, results)
    return (time_consume, conversion_time, total_framework_time, config)

def test_gpu(args, features, label, sklearnmodel, config, time_consume):
    import torch
    import gc
    gc.collect()
    with torch.no_grad():
        torch.cuda.empty_cache()
    input_size = len(label)
    if FRAMEWORK == "HummingbirdPytorchGPU":
        import torch
        device = torch.device('cuda')
        start_time = time.time()
        relative_path = relative2abspath("models", f"{DATASET}_{MODEL}_{config['num_trees']}_{config['depth']}_torch.pkl")
        model = convert_to_hummingbird_model(sklearnmodel, "torch", features, args.batch_size, "cuda")
        conversion_time = calculate_time(start_time, time.time())
        results = run_inference(FRAMEWORK, features, input_size, args.query_size, model.predict, time_consume)
        write_data(FRAMEWORK, results, time_consume)
        total_framework_time = calculate_time(start_time, time.time())

    elif FRAMEWORK == "HummingbirdTorchScriptGPU":
        import hummingbird.ml as hml
        start_time = time.time()
        torch_data = features[0:args.query_size]
        model = hml.convert(sklearnmodel, "torch.jit", torch_data,"cuda")
        conversion_time = calculate_time(start_time, time.time())
        def predict(batch):
            return model.predict(batch)

        results = run_inference(FRAMEWORK, features, input_size, args.query_size, predict, time_consume)
        write_data(FRAMEWORK, results, time_consume)
        total_framework_time = calculate_time(start_time, time.time())

    elif FRAMEWORK == "ONNXGPU":
        import onnxruntime as rt
        start_time = time.time()
        relative_path = relative2abspath("models",f"{DATASET}_{MODEL}_{config['num_trees']}_{config['depth']}.onnx")
        sess = rt.InferenceSession(relative_path,providers=['CUDAExecutionProvider'])
        input_name = sess.get_inputs()[0].name
        label_name = sess.get_outputs()[0].name
        conversion_time = calculate_time(start_time, time.time())
        def predict(batch):
            output = sess.run([label_name], {input_name:batch})[0]
            return output

        results = run_inference(FRAMEWORK, features, input_size, args.query_size, predict, time_consume)
        write_data(FRAMEWORK, results, time_consume)
        total_framework_time = calculate_time(start_time, time.time())
    
    elif FRAMEWORK == "HummingbirdTVMGPU":
        assert args.batch_size == args.query_size, "For TVM, batch_size must be equivalent to query_size"
        start_time = time.time()
        model = convert_to_hummingbird_model(sklearnmodel, "tvm", features, args.batch_size, "cuda")
        remainder_size = input_size % args.batch_size
        if remainder_size > 0:
            remainder_model = convert_to_hummingbird_model(sklearnmodel, "tvm", features, remainder_size, "cuda")
        conversion_time = calculate_time(start_time, time.time())
        def predict(batch, use_remainder_model):
            if use_remainder_model:
                return remainder_model.predict(batch)
            return model.predict(batch)

        results = run_inference(FRAMEWORK, features, input_size, args.query_size, predict, time_consume)
        write_data(FRAMEWORK, results, time_consume)
        total_framework_time = calculate_time(start_time, time.time())
    
    elif FRAMEWORK == "XGBoostGPU":
        start_time = time.time()
        #scikit-learn will use all data in a query as one batch  
        conversion_time = 0.0
        sklearnmodel.set_params(predictor="gpu_predictor")  # NOT safe!
        sklearnmodel.set_params(n_jobs=1)
        results = run_inference(FRAMEWORK, features, input_size, args.query_size, sklearnmodel.predict, time_consume)
        write_data(FRAMEWORK, results, time_consume) 
        total_framework_time = calculate_time(start_time, time.time())
    
    elif FRAMEWORK == "NvidiaFILGPU":
        from cuml import ForestInference
        start_time = time.time()
        model = None
        if MODEL == 'randomforest':
            model = ForestInference.load_from_sklearn(sklearnmodel,output_class=True, storage_type='auto')
        elif MODEL == "lightgbm":
            relative_path = relative2abspath("models",f"{DATASET}_{MODEL}_{config['num_trees']}_{config['depth']}.model")
            model = ForestInference.load(relative_path,output_class=True, storage_type='auto',model_type="lightgbm")
        elif MODEL == 'xgboost':
            relative_path = relative2abspath("models",f"{DATASET}_{MODEL}_{config['num_trees']}_{config['depth']}.model")
            model = ForestInference.load(relative_path,output_class=True, storage_type='auto')
        else:
            print(MODEL + " support will be added to "+ FRAMEWORK)
            exit()

        # model = ForestInference.load_from_sklearn(sklearnmodel,output_class=True, storage_type='auto')
        conversion_time = calculate_time(start_time, time.time())
        results = run_inference(FRAMEWORK, features, input_size, args.query_size, model.predict, time_consume)
        write_data(FRAMEWORK, results, time_consume)
        total_framework_time = calculate_time(start_time, time.time())

    else:
        raise ValueError(f"{FRAMEWORK} is not supported.")
    if args.task_type == "classification":
        find_accuracy(FRAMEWORK, label, results)
    else:
        find_MSE(FRAMEWORK, label, results)
    return (time_consume, conversion_time, total_framework_time, config)

def test_postprocess(time_consume, conversion_time, total_framework_time, config):
    # Print conversion time and total time used on framework
    print(f"Time Taken to convert {FRAMEWORK} model: {conversion_time}")
    print(f"TOTAL Time Taken for {FRAMEWORK}: {total_framework_time}")

    # Update output dictionary (time consumption at each step)
    time_consume["conversion time"] = conversion_time
    time_consume["total framework time"] = total_framework_time

    # Save output dictionary to csv
    filename_suffix = "GPU" if FRAMEWORK.endswith("GPU") else "CPU"
    num_trees, depth = config['num_trees'], config['depth']
    output_file_path = relative2abspath("results", f"{DATASET}_{num_trees}_{depth}_{filename_suffix}.csv")
    file_exists = os.path.isfile(output_file_path)
    with open (output_file_path, 'a') as csvfile:
        headers = list(time_consume.keys())
        writer = csv.DictWriter(csvfile, delimiter=',', lineterminator='\n',fieldnames=headers)
        if not file_exists:
            writer.writeheader()  # file doesn't exist yet, write a header
        writer.writerow(time_consume)


if __name__ ==  "__main__":
    print("\n\n\n==============EXPERIMENT STARTING=========================")
    config = json.load(open(relative2abspath("config.json")))
    args = parse_arguments(config)
    print("Trees",config['num_trees'])
    print("Depth",config['depth'])    
    time_consume = {
        "query size": args.query_size,
        "batch_size": args.batch_size,
        "model": MODEL,
        "framework": FRAMEWORK}
    features, label = load_data(config, time_consume)
    sklearnmodel = load_sklearn_model(config, time_consume)
    test(args, features, label, sklearnmodel, config, time_consume)
    print("==============EXPERIMENT ENDING=========================\n")
