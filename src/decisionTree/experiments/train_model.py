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
from skl2onnx import convert_sklearn
from skl2onnx.common.data_types import FloatTensorType
import torch
import os
from model_helper import *

DATASET = "higgs"
CLASSFIER = "randomforest"
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

train_start_time = time.time()
classifer = None
if CLASSFIER == 'randomforest':
    classifier = RandomForestClassifier(n_estimators = num_trees, max_depth=depth)
elif CLASSFIER == "xgboost":
    classifier = XGBClassifier(n_estimators = num_trees, max_depth=depth)

classifier.fit(df_train[x_col],df_train[y_col]) 
train_end_time = time.time()

print("Time taken to train the classifier "+str(calulate_time(train_start_time, train_end_time)))
print(classification_report(df_train[y_col],classifier.predict(df_train[x_col])))

#saving the model using joblib
joblib.dump(classifier, os.path.join("models",DATASET+"_"+CLASSFIER+"_"+str(num_trees)+"_"+str(depth)+".pkl"))

#converting to pytorch model using hummingbird
model = hml.convert(classifier, 'pytorch')
torch.save(model, os.path.join("models",DATASET+"_"+CLASSFIER+"_"+str(num_trees)+"_"+str(depth)+"_torch.pkl"))

#converting to ONNX model
initial_type = [('float_input', FloatTensorType([None, 28]))]
onx = convert_sklearn(classifier, initial_types=initial_type)
with open(os.path.join("models",DATASET+"_"+CLASSFIER+"_"+str(num_trees)+"_"+str(depth)+".onnx"), "wb") as f:
    f.write(onx.SerializeToString())

