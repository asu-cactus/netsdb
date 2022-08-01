import warnings

from yaml import load
warnings.filterwarnings('ignore')

from sklearn.ensemble import RandomForestClassifier, RandomForestRegressor
from sklearn import metrics
from xgboost import XGBClassifier, XGBRegressor
import joblib
import numpy as np
import time
import json
import os
import argparse
from model_helper import *

DATASET = "higgs"
MODEL = "xgboost"

def get_relative_path(path, *paths):
    return os.path.join(
        os.path.dirname(__file__),
        path,
        *paths
    )

def parse_arguments():
    global DATASET, MODEL
    parser = argparse.ArgumentParser(description='Arguments for train_model.')
    parser.add_argument("-d", "--dataset", type=str, choices=['higgs', 'airline', 'fraud', 'year', 'epsilon'],
        help="Dataset to be trained. Choose from ['higgs', 'airline', 'fraud', 'year', 'epsilon']")
    parser.add_argument("-m", "--model", type=str, choices=['randomforest', 'xgboost'],
        help="Model name. Choose from ['randomforest', 'xgboost']")
    args = parser.parse_args()
    if args.dataset:
        DATASET = args.dataset
    if args.model:
        MODEL = args.model

    print(f"DATASET: {DATASET}")
    print(f"MODEL: {MODEL}")
    return args
 

def load_data(config):
    y_col = config[DATASET]["y_col"]
    df_train = fetch_data(DATASET,config,"train").astype({y_col: int})
    print(f"Number of training examples: {len(df_train)}")
    return df_train

def train(config, df_train):
    # Prepare data
    y_col = config[DATASET]["y_col"]
    x_col = list(df_train.columns)
    x_col.remove(y_col)

    x = np.array(df_train[x_col])
    y = np.array(df_train[y_col])

    # Load model
    # The settings of the models are consistent with Hummingbird: https://github.com/microsoft/hummingbird/blob/main/benchmarks/trees/train.py
    if MODEL == 'randomforest':
        if config[DATASET]["type"] == "classification":
            ModelClass = RandomForestClassifier
        else:
            ModelClass = RandomForestRegressor
        model = ModelClass(
            n_estimators = config["num_trees"], 
            max_depth=config["depth"] ,
            verbose=0,
            n_jobs=-1
        )
    elif MODEL == "xgboost":
        task_spec_args = {}
        if config[DATASET]["type"] == "classification":
            ModelClass = XGBClassifier  
            task_spec_args["scale_pos_weight"] = len(y) / np.count_nonzero(y)
            task_spec_args["objective"] = "binary:logistic"
        elif config[DATASET]["type"] == "regression":
            ModelClass = XGBRegressor
            task_spec_args["objective"] = "reg:squarederror"
        else:
            raise ValueError("Task type in config.json must be one of ['classification', 'regression']")
        model = ModelClass(
            max_depth=config["depth"],
            n_estimators=config["num_trees"],
            max_leaves=256,
            learning_rate=0.1,
            tree_method="hist",
            reg_lambda=1,
            verbosity=0,
            n_jobs=-1,
            **task_spec_args
        )

    # Train model
    train_start_time = time.time()
    model.fit(x,y) 
    train_end_time = time.time()
    print(f"Time taken to train the model: {calulate_time(train_start_time, train_end_time)}")
        
    # Compute metrics
    if config[DATASET]["type"] == "classification":
        metrics_method = metrics.classification_report
    else:
        metrics_method = metrics.mean_squared_error
    print(metrics_method(df_train[y_col],model.predict(df_train[x_col])))

    # Save the model using joblib
    joblib_time_start = time.time()
    joblib.dump(model, get_relative_path("models", f"{DATASET}_{MODEL}_{config['num_trees']}_{config['depth']}.pkl"))
    joblib_time_end = time.time()
    print(f"Time taken to save model using joblib: {calulate_time(joblib_time_start, joblib_time_end)}")


if __name__ ==  "__main__":
    parse_arguments()
    config = json.load(open(get_relative_path("config.json")))
    df_train = load_data(config)
    train(config, df_train)