import warnings
from sklearn.utils import check_random_state
warnings.filterwarnings('ignore')

# from sklearn.ensemble import RandomForestClassifier
# from sklearn.metrics import classification_report
from xgboost import XGBClassifier
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
from model_helper import *


DATASET = "higgs"
CLASSIFIER = "xgboost"
FRAMEWORKS = None

def parse_arguments():
    global DATASET, CLASSIFIER, FRAMEWORKS
    parser = argparse.ArgumentParser(description='Arguments for train_model.')
    parser.add_argument("-d", "--dataset", type=str, choices=['higgs', 'airline', 'fraud', 'year', 'epsilon'],
        help="Dataset to be trained. Choose from ['higgs', 'airline', 'fraud', 'year', 'epsilon']")
    parser.add_argument("-c", "--classifier", type=str, choices=['randomforest', 'xgboost'],
        help="Classifier name. Choose from ['randomforest', 'xgboost']")
    parser.add_argument("-f", "--frameworks", type=str,
        help="Zero to multiple values from ['pytorch', 'torch', 'tf-df', 'onnx', 'treelite'], seperated by ','")
    args = parser.parse_args()
    if args.dataset:
        DATASET = args.dataset.lower()
    if args.classifier:
        CLASSIFIER = args.classifier.lower()
    if args.frameworks:
        framework_options = ['pytorch', 'torch', 'tf-df', 'onnx', 'treelite']
        for framework in args.frameworks.lower().split(","):
            if framework not in framework_options:
                raise ValueError(f"Framework {framework} is not supported. Choose from ['pytorch', 'torch', 'tf-df', 'onnx', 'treelite']")
        FRAMEWORKS = args.frameworks

    check_argument_conflicts(args)
    print(f"DATASET: {DATASET}")
    print(f"CLASSIFIER: {CLASSIFIER}")
    print(f"FRAMEWORKS: {FRAMEWORKS}")
    return args

def check_argument_conflicts(args):
    classifier = args.classifier.lower()
    frameworks = args.frameworks.lower().split(",")
    if ("tf-df" in frameworks or "tfdf" in frameworks) and classifier == "xgboost":
        raise ValueError("TF-DF models only supports randomforest algorithm, but does not support randomforest algorithm.")
    if "treelite" in frameworks and classifier == "randomforest":
        raise ValueError("TreeLite models only supports xgboost algorithm, but does not support randomforest algorithm.")

def convert_to_pytorch_model(classifier, num_trees, depth):
    #converting to pytorch model using hummingbird
    humming_pytorch_time_start = time.time()
    model = hml.convert(classifier, 'pytorch')
    humming_pytorch_time_end = time.time()
    print("Time Taken to convert HummingbirdPyTorch:",calulate_time(humming_pytorch_time_start, humming_pytorch_time_end))
    
    save_pytorch_time_start = time.time()
    model.save(os.path.join("models",DATASET+"_"+CLASSIFIER+"_"+str(num_trees)+"_"+str(depth)+"_pytorch.pkl"))
    save_pytorch_time_end = time.time()
    print("Time taken to save pytorch model "+str(calulate_time(save_pytorch_time_start, save_pytorch_time_end)))

def convert_to_torch_model(classifier, num_trees, depth):
    #converting to torch model using hummingbird
    humming_torch_time_start = time.time()
    model = hml.convert(classifier, 'torch')
    humming_torch_time_end = time.time()
    print("Time taken to convert to torch using hummingbird "+str(calulate_time(humming_torch_time_start, humming_torch_time_end)))
    
    save_torch_time_start = time.time()
    torch.save(model, os.path.join("models",DATASET+"_"+CLASSIFIER+"_"+str(num_trees)+"_"+str(depth)+"_torch.pkl"))
    save_torch_time_end = time.time()
    print("Time taken to save torch model "+str(calulate_time(save_torch_time_start, save_torch_time_end)))

def convert_to_tf_df_model(classifier, num_trees, depth):
    #converting to TF-DF model
    import tensorflow as tf
    import scikit_learn_model_converter

    if CLASSIFIER == "randomforest":
        tfdf_time_start = time.time()
        #model_path = "/home/ubuntu/netsdb/model-inference/decisionTree/experiments/models/higgs_randomforest_10_8.pkl"
        #loaded_model = joblib.load(model_path)
        tensorflow_model = scikit_learn_model_converter.convert(classifier,  intermediate_write_path="intermediate_path", )
        libpath = os.path.join("models", DATASET+"_"+CLASSIFIER+"_"+str(num_trees)+"_"+str(depth)+"_tfdf")
        tf.saved_model.save(obj=tensorflow_model, export_dir=libpath)
        tfdf_time_end = time.time()
        print("Time taken to save tfdf randomforest model "+str(calulate_time(tfdf_time_start, tfdf_time_end)))
    else:
        print(f"TF-DF models only supports randomforest algorithm, but does not support {CLASSIFIER} algorithm.")

def convert_to_onnx_model(classifier, num_trees, depth):
    #converting to ONNX model
    if CLASSIFIER == "randomforest":
        onnx_time_start = time.time()
        initial_type = [('float_input', FloatTensorType([None, 28]))]
        model_onnx = convert_sklearn(classifier,'pipeline_xgboost', initial_types=initial_type)
        onnx_time_end = time.time()
        print("Time taken to convert onnx using hummingbird "+str(calulate_time(onnx_time_start, onnx_time_end)))
        onnx_write_time_start = time.time()
        with open(os.path.join("models",DATASET+"_"+CLASSIFIER+"_"+str(num_trees)+"_"+str(depth)+".onnx"), "wb") as f:
            f.write(model_onnx.SerializeToString())
        onnx_write_time_end = time.time()
        print("Time taken to write onnx model "+str(calulate_time(onnx_write_time_start, onnx_write_time_end)))
    elif CLASSIFIER == "xgboost":
        onnx_time_start = time.time()
        update_registered_converter(
        XGBClassifier, 'XGBoostXGBClassifier',
        calculate_linear_classifier_output_shapes, convert_xgboost,
        options={'nocl': [True, False], 'zipmap': [True, False, 'columns']})
        model_onnx = convert_sklearn(
        classifier, 'pipeline_xgboost',
        [('input', FloatTensorType([None, 28]))],
        target_opset={'': 12, 'ai.onnx.ml': 2})
        onnx_time_end = time.time()
        print("Time taken to convert onnx using hummingbird "+str(calulate_time(onnx_time_start, onnx_time_end)))
        onnx_write_time_start = time.time()
        with open(os.path.join("models",DATASET+"_"+CLASSIFIER+"_"+str(num_trees)+"_"+str(depth)+".onnx"), "wb") as f:
            f.write(model_onnx.SerializeToString())
        onnx_write_time_end = time.time()
        print("Time taken to write onnx model "+str(calulate_time(onnx_write_time_start, onnx_write_time_end)))
        # import the model to treelite 


def convert_to_treelite_model(classifier, num_trees, depth):
    #converting to TreeLite model
    #Prerequisite: install treelite (https://treelite.readthedocs.io/en/latest/install.html)
    if CLASSIFIER == "xgboost":
        treelite_time_start = time.time()
        model_path = os.path.join("models", DATASET+"_"+CLASSIFIER+"_"+str(num_trees)+"_"+str(depth)+".model")
        classifier.save_model(model_path)
        treelite_model = treelite.Model.load(model_path, model_format='xgboost')
        toolchain = 'gcc'
        libpath = os.path.join("models", DATASET+"_"+CLASSIFIER+"_"+str(num_trees)+"_"+str(depth)+".so")
        treelite_model.export_lib(toolchain, libpath, verbose=True)
        treelite_time_end = time.time()
        print("Time taken to convert and write treelite model "+str(calulate_time(treelite_time_start, treelite_time_end)))

        #import onnxruntime as rt
        # sess = rt.InferenceSession(os.path.join("models",DATASET+"_"+CLASSIFIER+"_"+str(num_trees)+"_"+str(depth)+".onnx"),providers=['CPUExecutionProvider'])
        # pred_onx = sess.run(None, {"input": x[:5].astype(np.float32)})
        # print("predictions", pred_onx[0])
        # # print("predict_proba", pred_onx[1][:1])
    else:
        print(f"TreeLite models only supports xgboost algorithm, but does not support {CLASSIFIER} algorithm.")

def convert(classifier, num_trees, depth):
    if FRAMEWORKS is None:
        return

    frameworks = FRAMEWORKS.lower().split(",")
    if "pytorch" in frameworks:
        convert_to_pytorch_model(classifier, num_trees, depth)
    if "torch" in frameworks:
        convert_to_torch_model(classifier, num_trees, depth)
    if "tf-df" in frameworks or 'tfdf' in frameworks:
        convert_to_tf_df_model(classifier, num_trees, depth)
    if "onnx" in frameworks:
        convert_to_onnx_model(classifier, num_trees, depth)
    if "treelite" in frameworks:
        convert_to_treelite_model(classifier, num_trees, depth)


def load_model(num_trees, depth):

    model_path = os.path.join("models", DATASET+"_"+CLASSIFIER+"_"+str(num_trees)+"_"+str(depth)+".pkl")
    classifier = joblib.load(model_path)
    return classifier
    # classifier.save_model("./model.model")
    # treelite_model = treelite.Model.load("./model.model", model_format='xgboost')
    # toolchain = 'gcc'
    # libpath = os.path.join("models", DATASET+"_"+CLASSIFIER+"_"+str(num_trees)+"_"+str(depth)+"-1.so")
    # treelite_model.export_lib(toolchain=toolchain, libpath=libpath, params={'parallel_comp': 4}, verbose=True)

if __name__ ==  "__main__":
    parse_arguments()
    config = json.load(open("config.json"))
    num_trees = config["num_trees"]
    depth = config["depth"]
    classifier = load_model(num_trees, depth)
    convert(classifier, num_trees, depth)