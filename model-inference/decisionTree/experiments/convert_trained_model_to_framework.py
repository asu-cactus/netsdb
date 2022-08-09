import warnings
from sklearn.utils import check_random_state
warnings.filterwarnings('ignore')

# from sklearn.ensemble import RandomForestClassifier
# from sklearn.metrics import classification_report
from xgboost import XGBClassifier, XGBRegressor
import treelite
import treelite.sklearn
import joblib
import time
import json
import hummingbird.ml as hml
from skl2onnx.common.data_types import FloatTensorType
from skl2onnx import convert_sklearn, to_onnx, update_registered_converter
from skl2onnx.common.shape_calculator import (
    calculate_linear_classifier_output_shapes,
    calculate_linear_regressor_output_shapes)
from onnxmltools.convert.xgboost.operator_converters.XGBoost import convert_xgboost
import torch
import os
import argparse
from enum import Enum
from model_helper import *
from train_model import get_relative_path

class LearningTask(Enum):
    REGRESSION = 1
    CLASSIFICATION = 2

DATASET = "higgs"
MODEL = "xgboost"
FRAMEWORKS = None
LEARNINGTASK = LearningTask.CLASSIFICATION

def parse_arguments():
    global DATASET, MODEL, FRAMEWORKS, LEARNINGTASK
    parser = argparse.ArgumentParser(description='Arguments for train_model.')
    parser.add_argument("-d", "--dataset", type=str, choices=['higgs', 'airline', 'fraud', 'year', 'epsilon'],
        help="Dataset to be trained. Choose from ['higgs', 'airline', 'fraud', 'year', 'epsilon']")
    parser.add_argument("-m", "--model", type=str, choices=['randomforest', 'xgboost'],
        help="Model name. Choose from ['randomforest', 'xgboost']")
    parser.add_argument("-f", "--frameworks", type=str,
        help="Zero to multiple values from ['pytorch', 'torch', 'tf-df', 'onnx', 'treelite'], seperated by ','")
    args = parser.parse_args()
    if args.dataset:
        DATASET = args.dataset.lower()
    if args.model:
        MODEL = args.model.lower()
    if args.frameworks:
        framework_options = ['pytorch', 'torch', 'tf-df', 'onnx', 'treelite']
        for framework in args.frameworks.lower().split(","):
            if framework not in framework_options:
                raise ValueError(f"Framework {framework} is not supported. Choose from ['pytorch', 'torch', 'tf-df', 'onnx', 'treelite']")
        FRAMEWORKS = args.frameworks

    check_argument_conflicts(args)
    print(f"DATASET: {DATASET}")
    print(f"MODEL: {MODEL}")
    print(f"FRAMEWORKS: {FRAMEWORKS}")

    if DATASET == "year":
        LEARNINGTASK = LearningTask.REGRESSION

    return args

def check_argument_conflicts(args):
    model = args.model.lower()
    frameworks = args.frameworks.lower().split(",")
    if "treelite" in frameworks and model == "randomforest":
        raise ValueError("TreeLite models only supports xgboost algorithm, but does not support randomforest algorithm.")

def convert_to_pytorch_model(model, config):
    #converting to pytorch model using hummingbird
    humming_pytorch_time_start = time.time()
    model = hml.convert(model, 'pytorch')
    humming_pytorch_time_end = time.time()
    print("Time Taken to convert HummingbirdPyTorch:",calulate_time(humming_pytorch_time_start, humming_pytorch_time_end))
    
    save_pytorch_time_start = time.time()
    model.save(get_relative_path("models", f"{DATASET}_{MODEL}_{config['num_trees']}_{config['depth']}_pytorch.pkl"))
    save_pytorch_time_end = time.time()
    print("Time taken to save pytorch model "+str(calulate_time(save_pytorch_time_start, save_pytorch_time_end)))

def convert_to_torch_model(model, config):
    #converting to torch model using hummingbird
    humming_torch_time_start = time.time()
    model = hml.convert(model, 'torch')
    humming_torch_time_end = time.time()
    print("Time taken to convert to torch using hummingbird "+str(calulate_time(humming_torch_time_start, humming_torch_time_end)))
    
    save_torch_time_start = time.time()
    torch.save(model, get_relative_path("models", f"{DATASET}_{MODEL}_{config['num_trees']}_{config['depth']}_torch.pkl"))
    save_torch_time_end = time.time()
    print("Time taken to save torch model "+str(calulate_time(save_torch_time_start, save_torch_time_end)))

def convert_to_tf_df_model(model, config):
    #converting to TF-DF model
    import tensorflow as tf
    import scikit_learn_model_converter
    import xgboost_model_converter

    if MODEL == "randomforest":
        tfdf_time_start = time.time()
        #model_path = "/home/ubuntu/netsdb/model-inference/decisionTree/experiments/models/higgs_randomforest_10_8.pkl"
        #loaded_model = joblib.load(model_path)
        tensorflow_model = scikit_learn_model_converter.convert(model,  intermediate_write_path="intermediate_path", )
        libpath = get_relative_path("models", f"{DATASET}_{MODEL}_{config['num_trees']}_{config['depth']}_tfdf")
        tf.saved_model.save(obj=tensorflow_model, export_dir=libpath)
        tfdf_time_end = time.time()
        print("Time taken to save tfdf randomforest model "+str(calulate_time(tfdf_time_start, tfdf_time_end)))

    elif MODEL == "xgboost":
        tfdf_time_start = time.time()
        tensorflow_model = xgboost_model_converter.convert(model, intermediate_write_path="intermediate_path",)
        libpath = get_relative_path("models", f"{DATASET}_{MODEL}_{config['num_trees']}_{config['depth']}_tfdf")
        tf.saved_model.save(obj=tensorflow_model, export_dir=libpath)
        tfdf_time_end = time.time()
        print("Time taken to save tfdf xgboost model "+str(calulate_time(tfdf_time_start, tfdf_time_end)))


def convert_to_onnx_model(model, config):
    #converting to ONNX model
    if MODEL == "randomforest":
        onnx_time_start = time.time()
        initial_type = [('float_input', FloatTensorType([None, config[DATASET]['num_features']]))]
        model_onnx = convert_sklearn(model,'pipeline_xgboost', initial_types=initial_type)
        onnx_time_end = time.time()
        print("Time taken to convert onnx using hummingbird "+str(calulate_time(onnx_time_start, onnx_time_end)))
        
        onnx_write_time_start = time.time()
        with open(get_relative_path("models", f"{DATASET}+{MODEL}_{config['num_trees']}_{config['depth']}.onnx"), "wb") as f:
            f.write(model_onnx.SerializeToString())
        onnx_write_time_end = time.time()
        print("Time taken to write onnx model "+str(calulate_time(onnx_write_time_start, onnx_write_time_end)))
    elif MODEL == "xgboost":
        onnx_time_start = time.time()
        if LEARNINGTASK == LearningTask.CLASSIFICATION:
            ModelClass = XGBClassifier
            model_name = 'XGBoostXGBClassifier'
            shape_fct = calculate_linear_classifier_output_shapes
        else:
            ModelClass = XGBRegressor
            model_name = 'XGBoostXGBRegressor'
            shape_fct = calculate_linear_regressor_output_shapes
        update_registered_converter(
            ModelClass, 
            model_name,
            shape_fct, 
            convert_xgboost,
            options={'nocl': [True, False], 'zipmap': [True, False, 'columns']}
        )
        model_onnx = convert_sklearn(
            model, 
            'pipeline_xgboost',
            [('input', FloatTensorType([None, config[DATASET]['num_features']]))],
            target_opset={'': 12, 'ai.onnx.ml': 2}
        )
        onnx_time_end = time.time()
        print("Time taken to convert onnx using hummingbird "+str(calulate_time(onnx_time_start, onnx_time_end)))

        onnx_write_time_start = time.time()
        with open(get_relative_path("models", f"{DATASET}_{MODEL}_{config['num_trees']}_{config['depth']}.onnx"), "wb") as f:
            f.write(model_onnx.SerializeToString())
        onnx_write_time_end = time.time()
        print("Time taken to write onnx model "+str(calulate_time(onnx_write_time_start, onnx_write_time_end)))


def convert_to_treelite_model(model, config):
    #converting to TreeLite model
    #Prerequisite: install treelite (https://treelite.readthedocs.io/en/latest/install.html)
    if MODEL == "xgboost":
        treelite_time_start = time.time()
        model_path = get_relative_path("models", f"{DATASET}_{MODEL}_{config['num_trees']}_{config['depth']}.model")
        model.save_model(model_path)
        treelite_model = treelite.Model.load(model_path, model_format='xgboost')
        toolchain = 'gcc'
        libpath = get_relative_path("models", f"{DATASET}_{MODEL}_{config['num_trees']}_{config['depth']}.so")
        treelite_model.export_lib(toolchain, libpath, verbose=True, params={"parallel_comp":os.cpu_count()})
        treelite_time_end = time.time()
        print("Time taken to convert and write treelite model "+str(calulate_time(treelite_time_start, treelite_time_end)))

        #import onnxruntime as rt
        # sess = rt.InferenceSession(get_relative_path("models",DATASET+"_"+MODEL+"_"+str(num_trees)+"_"+str(depth)+".onnx"),providers=['CPUExecutionProvider'])
        # pred_onx = sess.run(None, {"input": x[:5].astype(np.float32)})
        # print("predictions", pred_onx[0])
        # # print("predict_proba", pred_onx[1][:1])
    else:
        print(f"TreeLite models only supports xgboost algorithm, but does not support {MODEL} algorithm.")

def convert(model, config):
    if FRAMEWORKS is None:
        return

    frameworks = FRAMEWORKS.lower().split(",")
    if "pytorch" in frameworks:
        convert_to_pytorch_model(model,config)
    if "torch" in frameworks:
        convert_to_torch_model(model, config)
    if "tf-df" in frameworks or 'tfdf' in frameworks:
        convert_to_tf_df_model(model, config)
    if "onnx" in frameworks:
        convert_to_onnx_model(model, config)
    if "treelite" in frameworks:
        convert_to_treelite_model(model, config)


def load_model(config):
    model = joblib.load(get_relative_path("models", f"{DATASET}_{MODEL}_{config['num_trees']}_{config['depth']}.pkl"))
    return model

if __name__ ==  "__main__":
    parse_arguments()
    config = json.load(open(get_relative_path("config.json")))
    model = load_model(config)
    convert(model, config)
