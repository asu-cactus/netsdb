from sklearn.ensemble import RandomForestClassifier
import pandas
import pickle
import os
import joblib
import pathlib
import psycopg2
import numpy as np
import time
import connectorx as cx
from sklearn import tree
import tensorflow as tf
import tensorflow_decision_forests as tfdf
import scikit_learn_model_converter

DATASET = "higgs"
CLASSIFIER = "randomforest"
num_trees = 10
depth = 8
model_path = os.path.join("/home/ubuntu/netsdb/model-inference/decisionTree/experiments/models", DATASET+"_"+CLASSIFIER+"_"+str(num_trees)+"_"+str(depth)+".pkl")
#model_path = "/home/ubuntu/netsdb/model-inference/decisionTree/experiments/models/higgs_randomforest_10_8.pkl"
loaded_model = joblib.load(model_path)
tensorflow_model = scikit_learn_model_converter.convert(loaded_model,  intermediate_write_path="intermediate_path", )
libpath = os.path.join("/home/ubuntu/netsdb/model-inference/decisionTree/experiments/models", DATASET+"_"+CLASSIFIER+"_"+str(num_trees)+"_"+str(depth)+"_tfdf")
tf.saved_model.save(obj=tensorflow_model, export_dir=libpath)

