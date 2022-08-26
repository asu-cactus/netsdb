import os
import sys
import json
from model_helper import relative2abspath
import numpy as np
import pandas as pd
if sys.version_info[0] >= 3:
    from urllib.request import urlretrieve
else:
    from urllib import urlretrieve
import pickle
from sklearn import datasets
from catboost.datasets import epsilon

dataset_folder = 'dataset/'

def download_data(url):
    local_url = relative2abspath(dataset_folder, os.path.basename(url))
    if not os.path.isfile(local_url):
        urlretrieve(url, local_url)
    print("Dataset downloaded.")
    return local_url


def prepare_airline(dataset_save_path, nrows=None):
    url = "http://kt.ijs.si/elena_ikonomovska/datasets/airline/airline_14col.data.bz2"
    local_url = download_data(url)
    
    cols = [
        "Year",
        "Month",
        "DayofMonth",
        "DayofWeek",
        "CRSDepTime",
        "CRSArrTime",
        "UniqueCarrier",
        "FlightNum",
        "ActualElapsedTime",
        "Origin",
        "Dest",
        "Distance",
        "Diverted",
        "ArrDelay",
    ]

    # load the data as int16
    dtype = np.int16

    dtype_columns = {
        "Year": dtype, "Month": dtype, "DayofMonth": dtype, "DayofWeek": dtype,
        "CRSDepTime": dtype, "CRSArrTime": dtype, "FlightNum": dtype,
        "ActualElapsedTime": dtype, "Distance": dtype,
        "Diverted": dtype, "ArrDelay": dtype,
    }
    df = pd.read_csv(local_url, names=cols, dtype=dtype_columns, nrows=nrows)
    
    print("Dataframe loaded.")
    # Encode categoricals as numeric
    for col in df.select_dtypes(['object']).columns:
        df[col] = df[col].astype("category").cat.codes

    print("Conversion complete.")
    if DATASET.endswith("classification"):
        # Turn into binary classification problem
        df["ArrDelay"] = 1 * (df["ArrDelay"] > 0)
    df.to_csv(dataset_save_path, index=False)


def prepare_bosch(dataset_save_path, nrows=None):
    filename = "train_numeric.csv.zip"
    local_url = relative2abspath(dataset_folder, filename)
    # local_url = os.path.join(dataset_folder, filename)
    if not os.path.isfile(local_url):
        os.system("kaggle competitions download -c bosch-production-line-performance -f " +
                filename + " -p " + dataset_folder)
    print("Downloaded bosch dataset.")

    df = pd.read_csv(local_url, index_col=0, compression='zip', dtype={"Response": np.int8}, nrows=nrows)
    df.to_csv(dataset_save_path, index=False)


def prepare_year(dataset_save_path, nrows=None):
    url = 'https://archive.ics.uci.edu/ml/machine-learning-databases/00203/YearPredictionMSD.txt.zip'
    # local_url = os.path.join(dataset_folder, os.path.basename(url))
    # if not os.path.isfile(local_url):
    #     urlretrieve(url, local_url)
    local_url = download_data(url)
    print("Downloaded year dataset.")

    df = pd.read_csv(local_url, header=None, nrows=nrows)
    df.to_csv(dataset_save_path, index=False)


def prepare_epsilon(dataset_save_path, nrows=None):
    train_data, test_data = epsilon()
    print("Downloaded epsilon dataset.")
    if nrows is not None:
        train_data = train_data[:nrows]
        test_data = test_data[:nrows]

    df = pd.concat([train_data, test_data], ignore_index=True)
    df.to_csv(dataset_save_path, index=False)


def prepare_covtype(dataset_save_path, nrows=None): 
    df = datasets.fetch_covtype(as_frame=True)["frame"]
    if nrows is not None:
        df = df[:nrows]
    df.to_csv(dataset_save_path, index=False)


if __name__ ==  "__main__":
    DATASET='airline_regression'
    nrows = 5000
    args = sys.argv
    if len(args) > 1:
        DATASET = args[-1]
    config = json.load(open(relative2abspath("config.json")))
    dataset_save_path = relative2abspath(dataset_folder, config[DATASET]["filename"])  
    if not os.path.exists(dataset_folder):
        os.makedirs(dataset_folder)
    if DATASET == 'year':
        prepare_year(dataset_save_path, nrows=nrows)
    elif DATASET == 'airline_regression' or DATASET == 'airline_classification':
        prepare_airline(dataset_save_path, nrows=nrows)
    elif DATASET == 'epsilon':
        prepare_epsilon(dataset_save_path, nrows=nrows)
    elif DATASET == 'bosch':
        prepare_bosch(dataset_save_path, nrows=nrows)
    elif DATASET == 'covtype':
        prepare_covtype(dataset_save_path, nrows=nrows)
    else:
        raise ValueError(f"{DATASET} not supported")

