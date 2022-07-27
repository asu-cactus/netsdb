import treelite
import treelite_runtime
import pickle
import xgboost
import gzip
import joblib
import pandas as pd
import treelite.sklearn
import os
from convert_scikitlearn_to_treelite import convert

DATASET="higgs"
CLASSIFIER="xgboost"
num_trees = 500
depth = 8
convert(DATASET, CLASSIFIER, num_trees, depth)
libpath = os.path.join("models", DATASET+"_"+CLASSIFIER+"_"+str(num_trees)+"_"+str(depth)+"-1.so")
predictor = treelite_runtime.Predictor(libpath, verbose=True)

test_data_path = 'HIGGS.csv_test.csv'
df = pd.read_csv(test_data_path) 
dmat = treelite_runtime.DMatrix(df.iloc[10:20, 1:28])
out_pred=predictor.predict(dmat)
print(out_pred)
