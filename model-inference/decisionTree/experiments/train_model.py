import warnings
warnings.filterwarnings('ignore')

from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import classification_report
from xgboost import XGBClassifier
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
CLASSFIER = "xgboost"
if len(args) == 3:
    DATASET = args[1]
    CLASSFIER = args[2] 

print(DATASET)
print(CLASSFIER)

config = json.load(open("config.json"))
datasetconfig = config[DATASET]
train_size = datasetconfig["train"]


df = fetch_data(DATASET,config)
df = df.astype({"label": int})
df_train = df.sample(frac=train_size)

num_trees = config["num_trees"]
depth = config["depth"]
y_col = datasetconfig["y_col"]
x_col = list(df_train.columns)
x_col.remove(y_col)

x = np.array(df_train[x_col])
y = np.array(df_train[y_col])

train_start_time = time.time()
classifer = None
if CLASSFIER == 'randomforest':
    classifier = RandomForestClassifier(n_estimators = num_trees, max_depth=depth)
elif CLASSFIER == "xgboost":
    classifier = XGBClassifier(n_estimators = num_trees, max_depth=depth)

classifier.fit(x,y) 
train_end_time = time.time()

print("Time taken to train the classifier "+str(calulate_time(train_start_time, train_end_time)))
print(classification_report(df_train[y_col],classifier.predict(df_train[x_col])))

#saving the model using joblib
joblib.dump(classifier, os.path.join("models",DATASET+"_"+CLASSFIER+"_"+str(num_trees)+"_"+str(depth)+".pkl"))

#converting to pytorch model using hummingbird
model = hml.convert(classifier, 'torch')
torch.save(model, os.path.join("models",DATASET+"_"+CLASSFIER+"_"+str(num_trees)+"_"+str(depth)+"_torch.pkl"))

#converting to ONNX model
if CLASSFIER == "randomforest":
    initial_type = [('float_input', FloatTensorType([None, 28]))]
    model_onnx = convert_sklearn(classifier,'pipeline_xgboost', initial_types=initial_type)
    with open(os.path.join("models",DATASET+"_"+CLASSFIER+"_"+str(num_trees)+"_"+str(depth)+".onnx"), "wb") as f:
        f.write(model_onnx.SerializeToString())
elif CLASSFIER == "xgboost":
    update_registered_converter(
    XGBClassifier, 'XGBoostXGBClassifier',
    calculate_linear_classifier_output_shapes, convert_xgboost,
    options={'nocl': [True, False], 'zipmap': [True, False, 'columns']})
    model_onnx = convert_sklearn(
    classifier, 'pipeline_xgboost',
    [('input', FloatTensorType([None, 28]))],
    target_opset={'': 12, 'ai.onnx.ml': 2})
    with open(os.path.join("models",DATASET+"_"+CLASSFIER+"_"+str(num_trees)+"_"+str(depth)+".onnx"), "wb") as f:
        f.write(model_onnx.SerializeToString())
    
    # import onnxruntime as rt
    # sess = rt.InferenceSession(os.path.join("models",DATASET+"_"+CLASSFIER+"_"+str(num_trees)+"_"+str(depth)+".onnx"),providers=['CPUExecutionProvider'])
    # pred_onx = sess.run(None, {"input": x[:5].astype(np.float32)})
    # print("predictions", pred_onx[0])
    # # print("predict_proba", pred_onx[1][:1])
