import treelite
import treelite_runtime
import pickle
import xgboost
import gzip
import joblib
import pandas as pd
import treelite.sklearn
import os

def convert (DATASET, CLASSIFIER, num_trees, depth):
    model_path = os.path.join("models", DATASET+"_"+CLASSIFIER+"_"+str(num_trees)+"_"+str(depth)+".pkl")
    model = joblib.load(model_path)
    model.save_model("./model.model")
    treelite_model = treelite.Model.load("./model.model", model_format='xgboost')
    toolchain = 'gcc'
    libpath = os.path.join("models", DATASET+"_"+CLASSIFIER+"_"+str(num_trees)+"_"+str(depth)+"-1.so")
    treelite_model.export_lib(toolchain=toolchain, libpath=libpath, params={'parallel_comp': 4}, verbose=True)

