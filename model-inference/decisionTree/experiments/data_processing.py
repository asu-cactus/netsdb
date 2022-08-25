import os
import sys
import numpy as np
import pandas as pd
if sys.version_info[0] >= 3:
    from urllib.request import urlretrieve
else:
    from urllib import urlretrieve


DATASET='airline'
IS_CLASSIFICATION=False
dataset_folder='./dataset'

def load_data(dataset_folder, url):
    local_url = os.path.join(dataset_folder, os.path.basename(url))
    if not os.path.isfile(local_url):
        urlretrieve(url, local_url)
    return local_url

def download_and_process_year(dataset_folder):
    url = "https://archive.ics.uci.edu/ml/machine-learning-databases/00203/YearPredictionMSD.txt" ".zip"
    load_data(dataset_folder, url)

def download_and_process_airline(database_folder, is_classification):
    url = "http://kt.ijs.si/elena_ikonomovska/datasets/airline/airline_14col.data.bz2"
    local_url = load_data(dataset_folder, url)
    
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
        "ActualElapsedTime": dtype, "Distance":
            dtype,
        "Diverted": dtype, "ArrDelay": dtype,
    }
    df = pd.read_csv(local_url,
                     names=cols, dtype=dtype_columns)
    
    # Encode categoricals as numeric
    for col in df.select_dtypes(['object']).columns:
        df[col] = df[col].astype("category").cat.codes


    if is_classification:
        # Turn into binary classification problem
        df["ArrDelay"] = 1 * (df["ArrDelay"] > 0)
        local_url = os.path.join(dataset_folder, 'airline-classification.csv')
    else:
        local_url = os.path.join(dataset_folder, 'airline-regression.csv')
    df.to_csv(local_url, index=False)
    
    


if __name__ ==  "__main__":
    if not os.path.exists(dataset_folder):
        os.makedirs(dataset_folder)
    if DATASET == 'year':
        download_and_process_year(dataset_folder)
    elif DATASET == 'airline':
        download_and_process_airline(dataset_folder, IS_CLASSIFICATION)
    else:
        print(DATASET, " not supported")

