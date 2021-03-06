import warnings
warnings.filterwarnings('ignore')

from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import classification_report
from xgboost import XGBClassifier
import treelite
import treelite_runtime
import treelite.sklearn
import pandas
import pickle
import joblib
import numpy as np
import time
import json
import hummingbird.ml as hml
from skl2onnx.common.data_types import FloatTensorType
from skl2onnx import convert_sklearn, to_onnx, update_registered_converter
from skl2onnx.common.shape_calculator import (
    calculate_linear_classifier_output_shapes,
    calculate_linear_regressor_output_shapes)
from onnxmltools.convert.xgboost.operator_converters.XGBoost import (
    convert_xgboost)
from onnxmltools.convert import convert_xgboost as convert_xgboost_booster
import torch
import os
import sys
from model_helper import *

args = sys.argv

DATASET = "higgs"
CLASSIFIER = "xgboost"

if len(args) == 3:
    DATASET = args[1]
    CLASSIFIER = args[2] 

print(DATASET)
print(CLASSIFIER)

config = json.load(open("config.json"))
datasetconfig = config[DATASET]
train_size = datasetconfig["train"]


df = fetch_data(DATASET,config,"train")
df_train = df.astype({"label": int})
# df_train = df.sample(frac=train_size)
print(len(df_train))

num_trees = config["num_trees"]
depth = config["depth"]
y_col = datasetconfig["y_col"]
x_col = list(df_train.columns)
x_col.remove(y_col)

x = np.array(df_train[x_col])
y = np.array(df_train[y_col])

train_start_time = time.time()
classifer = None
if CLASSIFIER == 'randomforest':
    classifier = RandomForestClassifier(n_estimators = num_trees, max_depth=depth,verbose=3,n_jobs=-1)
elif CLASSIFIER == "xgboost":
    classifier = XGBClassifier(n_estimators = num_trees, max_depth=depth,verbosity=3,n_jobs=-1)



classifier.fit(x,y) 
train_end_time = time.time()

print("Time taken to train the classifier "+str(calulate_time(train_start_time, train_end_time)))
print(classification_report(df_train[y_col],classifier.predict(df_train[x_col])))

#saving the model using joblib
joblib_time_start = time.time()
joblib.dump(classifier, os.path.join("models",DATASET+"_"+CLASSIFIER+"_"+str(num_trees)+"_"+str(depth)+".pkl"))
joblib_time_end = time.time()
print("Time taken to save classifier using joblib "+str(calulate_time(joblib_time_start, joblib_time_end)))

#converting to pytorch model using hummingbird
humming_pytorch_time_start = time.time()
model = hummingbird.ml.convert(classifier, 'pytorch')
humming_pytorch_time_end = time.time()
print("Time Taken to convert HummingbirdPyTorch:",calulate_time(humming_pytorch_time_start, humming_pytorch_time_end))
save_pytorch_time_start = time.time()
model.save(os.path.join("models",DATASET+"_"+CLASSIFIER+"_"+str(num_trees)+"_"+str(depth)+"_pytorch.pkl"))
save_pytorch_time_end = time.time()
print("Time taken to save pytorch model "+str(calulate_time(save_pytorch_time_start, save_pytorch_time_end)))

#converting to torch model using hummingbird
humming_torch_time_start = time.time()
model = hml.convert(classifier, 'torch')
humming_torch_time_end = time.time()
print("Time taken to convert to torch using hummingbird "+str(calulate_time(humming_torch_time_start, humming_torch_time_end)))
save_torch_time_start = time.time()
torch.save(model, os.path.join("models",DATASET+"_"+CLASSIFIER+"_"+str(num_trees)+"_"+str(depth)+"_torch.pkl"))
save_torch_time_end = time.time()
print("Time taken to save torch model "+str(calulate_time(save_torch_time_start, save_torch_time_end)))

#converting to TF-DF model
if CLASSIFIER == "randomforest":
    tfdf_time_start = time.time()
    #model_path = "/home/ubuntu/netsdb/model-inference/decisionTree/experiments/models/higgs_randomforest_10_8.pkl"
    #loaded_model = joblib.load(model_path)
    tensorflow_model = scikit_learn_model_converter.convert(classifier,  intermediate_write_path="intermediate_path", )
    libpath = os.path.join("models", DATASET+"_"+CLASSIFIER+"_"+str(num_trees)+"_"+str(depth)+"_tfdf")
    tf.saved_model.save(obj=tensorflow_model, export_dir=libpath)
    tfdf_time_end = time.time()
    print("Time taken to save tfdf randomforest model "+str(calulate_time(tfdf_time_start, tfdf_time_end)))


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

#converting to TreeLite model
#Prerequisite: install treelite (https://treelite.readthedocs.io/en/latest/install.html)
if CLASSIFIER == "xgboost":
    treelite_time_begin = time.time()
    model_path = os.path.join("models", DATASET+"_"+CLASSIFIER+"_"+str(num_trees)+"_"+str(depth)+".model")
    classifier.save_model(model_path)
    treelite_model = treelite.Model.load(model_path, model_format='xgboost')
    toolchain = 'gcc'
    libpath = os.path.join("models", DATASET+"_"+CLASSIFIER+"_"+str(num_trees)+"_"+str(depth)+".so")
    treelite_model.export_lib(toolchain=toolchain, libpath, verbose=True)
    treelite_time_end = time.time()
    print("Time taken to convert and write treelite model "+str(calulate_time(treelite_time_start, treelite_time_end)))

    #import onnxruntime as rt
    # sess = rt.InferenceSession(os.path.join("models",DATASET+"_"+CLASSIFIER+"_"+str(num_trees)+"_"+str(depth)+".onnx"),providers=['CPUExecutionProvider'])
    # pred_onx = sess.run(None, {"input": x[:5].astype(np.float32)})
    # print("predictions", pred_onx[0])
    # # print("predict_proba", pred_onx[1][:1])
