import os
import joblib
import xgboost as xgb
from xgboost import XGBClassifier
from model_helper import *

models = os.listdir('models')
for model_file in models:
    new_model = model_file.split('.')[0]+'.model'
    if model_file.endswith(".pkl") and 'xgboost' in model_file and new_model not in models:
        clf = joblib.load(relative2abspath('models',model_file))
        clf.save_model(relative2abspath('models',model_file.split('.')[0]+'.model'))
        print(model_file.split('.')[0]+'.model')
