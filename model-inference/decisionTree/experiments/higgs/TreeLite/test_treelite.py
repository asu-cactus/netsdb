import treelite
import treelite_runtime
import pickle
import xgboost
import gzip
import joblib
import pandas as pd
import treelite.sklearn
from convert_scikitlearn_to_treelite import convert

convert("higgs", "xgboost", 512, 8)
libpath = os.path.join("models", DATASET+"_"+CLASSIFIER+"_"+str(num_trees)+"_"+str(depth)+".so")
predictor = treelite_runtime.Predictor(libpath, verbose=True)

df = pd.read_csv(test_data_path) 
dmat = treelite_runtime.DMatrix(df.iloc[10:20, 1:28])
out_pred=predictor.predict(dmat)
print(out_pred)
