import warnings
warnings.filterwarnings('ignore')

# from sklearn.ensemble import RandomForestClassifier
# from sklearn.metrics import classification_report
import xgboost
from xgboost import XGBClassifier, XGBRegressor
import treelite
import treelite.sklearn
import joblib
import time
import json
import hummingbird.ml as hml
from skl2onnx.common.data_types import FloatTensorType
from skl2onnx import convert_sklearn
import onnxmltools
import torch
import os
import argparse
from model_helper import *


DATASET = "higgs"
MODEL = "xgboost"
FRAMEWORKS = None

def parse_arguments(config):
    # check_argument_conflicts(args)  # TODO: Move this function from the bottom to here, after checking with Prof.
    global DATASET, MODEL, FRAMEWORKS
    parser = argparse.ArgumentParser(description='Arguments for train_model.')
    parser.add_argument("-d", "--dataset", type=str, choices=['higgs', 'airline_classification', 'airline_regression', 'fraud', 'year', 'epsilon', 'bosch', 'covtype', 'tpcxai_fraud'],
        help="Dataset to be trained. Choose from ['higgs', 'airline_classification', 'airline_regression', 'fraud', 'year', 'epsilon', 'bosch', 'covtype', 'tpcxai_fraud']")
    parser.add_argument("-m", "--model", type=str, choices=['randomforest', 'xgboost', 'lightgbm'],
        help="Model name. Choose from ['randomforest', 'xgboost', 'lightgbm']")
    parser.add_argument("-f", "--frameworks", type=str,
        help="Zero to multiple values from ['pytorch', 'torch', 'tf-df', 'onnx', 'treelite', 'lleaves', 'netsdb'], seperated by ','")
    parser.add_argument("-t", "--num_trees", type=int,
        help="Number of trees for the model")
    args = parser.parse_args()
    if args.dataset:
        DATASET = args.dataset.lower()
    if args.model:
        MODEL = args.model.lower()
    if args.frameworks:
        framework_options = ['pytorch', 'torch', 'tf-df', 'onnx', 'treelite', 'lleaves', 'netsdb']
        for framework in args.frameworks.lower().split(","):
            if framework not in framework_options:
                raise ValueError(f"Framework {framework} is not supported. Choose from ['pytorch', 'torch', 'tf-df', 'onnx', 'treelite', 'lleaves', 'netsdb']")
        FRAMEWORKS = args.frameworks  # TODO: Better to store these as a List? Instead of as a string.
    if args.num_trees:
        config["num_trees"] = args.num_trees
    check_argument_conflicts(args)  # TODO: Maybe, this is good to do it at the beginning of function itself?
    print(f"DATASET: {DATASET}")
    print(f"MODEL: {MODEL}")
    print(f"FRAMEWORKS: {FRAMEWORKS}")

    return args

def convert_to_pytorch_model(model, config):
    #converting to pytorch model using hummingbird
    humming_pytorch_time_start = time.time()
    model = hml.convert(model, 'pytorch')
    humming_pytorch_time_end = time.time()
    print("Time Taken to convert HummingbirdPyTorch:",calculate_time(humming_pytorch_time_start, humming_pytorch_time_end))
    
    save_pytorch_time_start = time.time()
    model.save(relative2abspath("models", f"{DATASET}_{MODEL}_{config['num_trees']}_{config['depth']}_pytorch.pkl"))
    save_pytorch_time_end = time.time()
    print("Time taken to save pytorch model "+str(calculate_time(save_pytorch_time_start, save_pytorch_time_end)))

def convert_to_torch_model(model, config):
    #converting to torch model using hummingbird
    humming_torch_time_start = time.time()
    model = hml.convert(model, 'torch')
    humming_torch_time_end = time.time()
    print("Time taken to convert to torch using hummingbird "+str(calculate_time(humming_torch_time_start, humming_torch_time_end)))
    
    save_torch_time_start = time.time()
    torch.save(model, relative2abspath("models", f"{DATASET}_{MODEL}_{config['num_trees']}_{config['depth']}_torch.pkl"))
    save_torch_time_end = time.time()
    print("Time taken to save torch model "+str(calculate_time(save_torch_time_start, save_torch_time_end)))

def convert_to_tf_df_model(model, config):
    # Converting to TF-DF model
    import tensorflow as tf
    import scikit_learn_model_converter  # TODO: Can we rename this file or move it, so that it is clear this is only meant for TFDF, and not used anywhere else.
    import xgboost_model_converter  # TODO: Can we rename this file or move it, so that it is clear this is only meant for TFDF, and not used anywhere else.

    if MODEL == "randomforest":
        tfdf_time_start = time.time()
        tensorflow_model = scikit_learn_model_converter.convert(model,  intermediate_write_path="intermediate_path", )
        libpath = relative2abspath("models", f"{DATASET}_{MODEL}_{config['num_trees']}_{config['depth']}_tfdf")
        tf.saved_model.save(obj=tensorflow_model, export_dir=libpath)
        tfdf_time_end = time.time()
        print("Time taken to save tfdf randomforest model "+str(calculate_time(tfdf_time_start, tfdf_time_end)))

    elif MODEL == "xgboost":
        tfdf_time_start = time.time()
        tensorflow_model = xgboost_model_converter.convert(model, intermediate_write_path="intermediate_path",)
        libpath = relative2abspath("models", f"{DATASET}_{MODEL}_{config['num_trees']}_{config['depth']}_tfdf")
        tf.saved_model.save(obj=tensorflow_model, export_dir=libpath)
        tfdf_time_end = time.time()
        print("Time taken to save tfdf xgboost model "+str(calculate_time(tfdf_time_start, tfdf_time_end)))


def convert_to_onnx_model(model, config):
    #converting to ONNX model
    if MODEL == "randomforest":
        onnx_time_start = time.time()
        initial_type = [('float_input', FloatTensorType([None, config[DATASET]['num_features']]))]
        model_onnx = convert_sklearn(model,'pipeline_xgboost', initial_types=initial_type)  # TODO: xgboost?
        onnx_time_end = time.time()
        print("Time taken to convert onnx using hummingbird "+str(calculate_time(onnx_time_start, onnx_time_end)))  # TODO: How is this hummingbird?
        
        onnx_write_time_start = time.time()
        with open(relative2abspath("models", f"{DATASET}_{MODEL}_{config['num_trees']}_{config['depth']}.onnx"), "wb") as f:
            f.write(model_onnx.SerializeToString())
        onnx_write_time_end = time.time()
        print("Time taken to write onnx model "+str(calculate_time(onnx_write_time_start, onnx_write_time_end)))
    elif MODEL == "xgboost":
        onnx_time_start = time.time()
        initial_types = [('float_input', FloatTensorType([None, config[DATASET]['num_features']]))]
        onnx_model = onnxmltools.convert_xgboost(model, initial_types=initial_types)
        onnx_time_end = time.time()
        print("Time taken to convert onnx using hummingbird "+str(calculate_time(onnx_time_start, onnx_time_end)))  # TODO: How is this hummingbird?

        onnx_write_time_start = time.time()
        model_save_path = relative2abspath("models", f"{DATASET}_{MODEL}_{config['num_trees']}_{config['depth']}.onnx")
        onnxmltools.utils.save_model(onnx_model, model_save_path)
        onnx_write_time_end = time.time()
        print("Time taken to write onnx model "+str(calculate_time(onnx_write_time_start, onnx_write_time_end)))
    elif MODEL == "lightgbm":
        onnx_time_start = time.time()  # TODO: Should we include parameter initialization in the time? Even though, it is negligible.
        initial_types = [('float_input', FloatTensorType([None, config[DATASET]['num_features']]))]
        onnx_model = onnxmltools.convert_lightgbm(model, initial_types=initial_types)
        onnx_time_end = time.time()
        print("Time taken to convert onnx using onnxmltools "+str(calculate_time(onnx_time_start, onnx_time_end)))

        onnx_write_time_start = time.time()
        model_save_path = relative2abspath("models", f"{DATASET}_{MODEL}_{config['num_trees']}_{config['depth']}.onnx")
        onnxmltools.utils.save_model(onnx_model, model_save_path)
        onnx_write_time_end = time.time()
        print("Time taken to write onnx model "+str(calculate_time(onnx_write_time_start, onnx_write_time_end)))



def convert_to_treelite_model(model, config):
    #converting to TreeLite model
    #Prerequisite: install treelite (https://treelite.readthedocs.io/en/latest/install.html)
    if MODEL in {"xgboost", "lightgbm"}:
        treelite_time_start = time.time()
        model_path = relative2abspath("models", f"{DATASET}_{MODEL}_{config['num_trees']}_{config['depth']}.model")
        if MODEL == "lightgbm":  # TODO: Rewrite this Logic.
            model.booster_.save_model(model_path)
        else:
            model.save_model(model_path)  # TODO: Why are we saving this directly to a .model format?
        treelite_model = treelite.Model.load(model_path, model_format=MODEL)
        toolchain = 'gcc'
        libpath = relative2abspath("models", f"{DATASET}_{MODEL}_{config['num_trees']}_{config['depth']}.so")
        treelite_model.export_lib(toolchain, libpath, verbose=True, params={"parallel_comp":os.cpu_count()})
        treelite_time_end = time.time()
        print("Time taken to convert and write treelite model "+str(calculate_time(treelite_time_start, treelite_time_end)))

    else:
        print(f"TreeLite only supports conversion from xgboost, lightgbm and sk-learn based models. Does not support {MODEL}.")


def convert_to_lleaves_model(model, config):
    # TODO: Option 1: Read pkl, write to txt, read txt model file from lleaves package.
    # TODO: Option 2: Write txt during model saving itself in train_model.py, and read it.
    # Implementing Option1 as it can be a drop-in to existing implmentation
    import lleaves
    if MODEL == 'lightgbm':
        lleaves_start_time = time.time()
        model_path = relative2abspath("models", f"{DATASET}_{MODEL}_{config['num_trees']}_{config['depth']}.txt")
        model.booster_.save_model(model_path)
        lleaves_model = lleaves.Model(model_file=model_path)
        model_cache_path = relative2abspath("models", f"{DATASET}_{MODEL}_{config['num_trees']}_{config['depth']}.elf")
        lleaves_model.compile(cache=model_cache_path)  # NOTE: Same logic to be used for testing. This time, the elf file is loaded instead of compiled.
        lleaves_time_end = time.time()
        print("Time taken to convert, compile and write Lleaves model "+str(calculate_time(lleaves_start_time, lleaves_time_end)))
    else:
        print(f"LLeaves is only supported for LightGBM at the moment. Does not support {MODEL}.")



def convert_to_netsdb_model(model, config):
    #converting to netsDB model
    from sklearn.tree import export_graphviz
    import graphviz
    import os
    
    netsdb_model_path = os.path.join("models", f"{DATASET}_{MODEL}_{config['num_trees']}_{config['depth']}_netsdb")

    if os.path.exists(netsdb_model_path) == False:
        os.mkdir(netsdb_model_path)
    
    if MODEL == "randomforest":
        estimators = model.estimators_

        for index, model in enumerate(estimators):
            output_file_path = os.path.join(netsdb_model_path, str(index)+'.txt')
            data = export_graphviz(model)
            f = open(output_file_path, 'w')
            f.write(data) 
            f.close()

    elif MODEL == "xgboost":
        num_trees = config['num_trees']
        
        for index in range(num_trees):
            output_file_path = os.path.join(netsdb_model_path, str(index)+'.txt') 
            data = xgboost.to_graphviz(model, num_trees=index)
            f = open(output_file_path, 'w')
            f.write(str(data)) 
            f.close()



def convert(model, config):
    def print_logs(function,model,config,framework_name):
        border = '-'*30
        print(border)
        print(f'Converting model to {framework_name}...')
        print(border)
        function(model,config)
        print(border)
        print(f'Converted model to {framework_name}')
        print(border + '\n\n')
    if FRAMEWORKS is None:
        return

    frameworks = FRAMEWORKS.lower().split(",")
    if "pytorch" in frameworks:
        print_logs(convert_to_pytorch_model,model,config,"PyTorch")
    if "torch" in frameworks:
        print_logs(convert_to_torch_model,model,config,"Torch")
    if "tf-df" in frameworks or 'tfdf' in frameworks:
        print_logs(convert_to_tf_df_model,model,config,"TF-DF")
    if "onnx" in frameworks:
        print_logs(convert_to_onnx_model,model,config,"ONNX")
    if "treelite" in frameworks:
        print_logs(convert_to_treelite_model,model,config,"TreeLite")
    if "lleaves" in frameworks:
        print_logs(convert_to_lleaves_model,model,config,"Lleaves")
    if "netsdb" in frameworks:
        print_logs(convert_to_netsdb_model, model, config, "netsdb")


def load_model(config):
    model = joblib.load(relative2abspath("models", f"{DATASET}_{MODEL}_{config['num_trees']}_{config['depth']}.pkl"))
    return model

if __name__ ==  "__main__":
    config = json.load(open(relative2abspath("config.json")))
    parse_arguments(config)
    model = load_model(config)
    convert(model, config)
