import treelite
import treelite_runtime
import pickle
import xgboost
import gzip
import joblib
import pandas as pd
import treelite.sklearn

model_path = "/home/ubuntu/netsdb/model-inference/decisionTree/experiments/models/higgs_xgboost_10_8.pkl" 
test_data_path = "/home/ubuntu/netsdb/model-inference/decisionTree/experiments/HIGGS.csv_test.csv"
model = joblib.load(model_path)
model.save_model("./model.model")
treelite_model = treelite.Model.load("./model.model", model_format='xgboost')
#treelite_model = treelite.Model.from_xgboost(model)
#treelite_model = treelite.sklearn.import_model(model)
toolchain = 'gcc'
treelite_model.compile(dirpath='./mymodel', verbose=True)
treelite_model.export_lib(toolchain=toolchain, libpath='./higgs_treelite_xgboost.so', verbose=True)
predictor = treelite_runtime.Predictor('./higgs_treelite_xgboost.so', verbose=True)

df = pd.read_csv(test_data_path) 
dmat = treelite_runtime.DMatrix(df.iloc[10:20, 1:28])
out_pred=predictor.predict(dmat)
print(out_pred)
