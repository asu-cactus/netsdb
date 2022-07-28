import warnings
warnings.filterwarnings('ignore')

from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import classification_report
from xgboost import XGBClassifier
import joblib
import numpy as np
import time
import json
import os
import argparse
from model_helper import *

DATASET = "higgs"
CLASSIFIER = "xgboost"

def parse_arguments():
    global DATASET, CLASSIFIER
    parser = argparse.ArgumentParser(description='Arguments for train_model.')
    parser.add_argument("-d", "--dataset", type=str, choices=['higgs', 'airline', 'fraud', 'year', 'epsilon'],
        help="Dataset to be trained. Choose from ['higgs', 'airline', 'fraud', 'year', 'epsilon']")
    parser.add_argument("-c", "--classifier", type=str, choices=['randomforest', 'xgboost'],
        help="Classifier name. Choose from ['randomforest', 'xgboost']")
    args = parser.parse_args()
    if args.dataset:
        DATASET = args.dataset
    if args.classifier:
        CLASSIFIER = args.classifier

    print(f"DATASET: {DATASET}")
    print(f"CLASSIFIER: {CLASSIFIER}")
    return args
 

def load_data(config):
    y_col = config[DATASET]["y_col"]
    df_train = fetch_data(DATASET,config,"train").astype({y_col: int})
    print(f"Number of training examples: {len(df_train)}")
    return df_train

def train(config, df_train):
    num_trees = config["num_trees"]
    depth = config["depth"]
    y_col = config[DATASET]["y_col"]
    x_col = list(df_train.columns)
    x_col.remove(y_col)

    x = np.array(df_train[x_col])
    y = np.array(df_train[y_col])

    train_start_time = time.time()
    if CLASSIFIER == 'randomforest':
        classifier = RandomForestClassifier(n_estimators = num_trees, max_depth=depth,verbose=3,n_jobs=-1)
    elif CLASSIFIER == "xgboost":
        classifier = XGBClassifier(n_estimators = num_trees, max_depth=depth,verbosity=3,n_jobs=-1)
    else:
        raise ValueError(f"Classifier {CLASSIFIER} is not supported, please choose from ['randomforest', 'xgboost]")
    classifier.fit(x,y) 
    train_end_time = time.time()

    print("Time taken to train the classifier "+str(calulate_time(train_start_time, train_end_time)))
    print(classification_report(df_train[y_col],classifier.predict(df_train[x_col])))

    #saving the model using joblib
    joblib_time_start = time.time()
    joblib.dump(classifier, os.path.join("models",DATASET+"_"+CLASSIFIER+"_"+str(num_trees)+"_"+str(depth)+".pkl"))
    joblib_time_end = time.time()
    print("Time taken to save classifier using joblib "+str(calulate_time(joblib_time_start, joblib_time_end)))

    return (classifier, num_trees, depth)


if __name__ ==  "__main__":
    parse_arguments()
    config = json.load(open("config.json"))
    df_train = load_data(config)
    train(config, df_train)