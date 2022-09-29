import os
import gc
import json
import pickle
from model_helper import relative2abspath, dataset_folder
import numpy as np
import pandas as pd
from urllib.request import urlretrieve
import psycopg2
from sklearn import datasets
from sklearn.utils import shuffle
import argparse
import math
import sys
import time


def parse_arguments():
    parser = argparse.ArgumentParser(
        description='Arguments for data_processing.py')
    parser.add_argument("-d", "--dataset", type=str, required=True,
                        choices=['higgs', 'airline_regression', 'airline_classification',
                                 'fraud', 'year', 'epsilon', 'bosch', 'covtype'],
                        help="Dataset to be processed. Choose from ['higgs', 'airline_regression', 'airline_classification', 'fraud', 'year', 'epsilon', 'bosch', 'covtype']")
    parser.add_argument("-n", "--nrows", type=int,
                        help="Load nrows of the dataset. Warning: only use in development.")
    args = parser.parse_args()
    return args


def download_data(url, dataset_folder):
    start_time = 0

    def reporthook(count, block_size, total_size):
        nonlocal start_time
        if count == 0:
            start_time = time.time()
            return
        duration = time.time() - start_time
        progress_size = int(count * block_size)
        speed = int(progress_size / (1024 * duration))
        percent = int(count * block_size * 100 / total_size)
        sys.stdout.write("\r...%d%%, %d MB, %d KB/s, %d seconds passed" %
                         (percent, progress_size / (1024 * 1024), speed, duration))
        sys.stdout.flush()

    local_url = relative2abspath(dataset_folder, os.path.basename(url))
    if not os.path.isfile(local_url):
        urlretrieve(url, local_url, reporthook)
    print("Dataset downloaded.")
    return local_url


def prepare_higgs(dataset_folder, nrows):
    url = 'https://archive.ics.uci.edu/ml/machine-learning-databases/00280/HIGGS.csv.gz'
    local_url = relative2abspath(dataset_folder, os.path.basename(url))
    download_data(url, dataset_folder)
    df = pd.read_csv(local_url, dtype=np.float32, header=None, nrows=nrows)
    df = df.astype({0: np.int8})
    return df


def prepare_airline(is_classification, dataset_folder, nrows=None):
    print("LOADING AIRLINE DATASET")
    url = "http://kt.ijs.si/elena_ikonomovska/datasets/airline/airline_14col.data.bz2"
    local_url = download_data(url, dataset_folder)

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
    if is_classification:
        # Turn into binary classification problem
        df["ArrDelay"] = 1 * (df["ArrDelay"] > 0)

    return df


def prepare_fraud(dataset_folder, nrows=None):
    filename = "creditcard.csv"
    local_url = relative2abspath(dataset_folder, filename)
    os.system("kaggle datasets download mlg-ulb/creditcardfraud -f" +
              filename + " -p " + dataset_folder)
    df = pd.read_csv(local_url + ".zip", dtype=np.float32, nrows=nrows)
    df = df.astype({"Class": np.int8})
    return df


def prepare_bosch(dataset_folder, nrows=None):
    filename = "train_numeric.csv.zip"
    local_url = relative2abspath(dataset_folder, filename)
    # local_url = os.path.join(dataset_folder, filename)
    if not os.path.isfile(local_url):
        os.system("kaggle competitions download -c bosch-production-line-performance -f " +
                  filename + " -p " + dataset_folder)
    print("Downloaded bosch dataset.")
    df = pd.read_csv(local_url, index_col=0, compression='zip',
                     dtype=np.float32, nrows=nrows)
    df = df.astype({"Response": np.int8})
    return df


def prepare_year(dataset_folder, nrows=None):
    url = 'https://archive.ics.uci.edu/ml/machine-learning-databases/00203/YearPredictionMSD.txt.zip'
    local_url = download_data(url, dataset_folder)
    df = pd.read_csv(local_url, dtype=np.float32, nrows=nrows, header=None)
    df = df.astype({0: np.int8})
    return df


def prepare_epsilon(nrows=None):
    from catboost.datasets import epsilon
    print("DOWNLOADING EPSILON")
    train_data, test_data = epsilon()
    # Training classifier requires labels [0 1], but the original data's labels are [-1 1]
    test_data[0][test_data[0] <= 0] = 0
    train_data[0][train_data[0] <= 0] = 0
    test_data = test_data.astype({0: np.int8})
    train_data = train_data.astype({0: np.int8})
    print("Downloaded epsilon dataset.")
    if nrows is not None:
        train_data = train_data[:nrows//2]
        test_data = test_data[:nrows//2]

    print(len(test_data))
    print(len(train_data))
    print("CONCATENATING TRAIN AND TEST")
    # df = pd.concat([train_data, test_data], ignore_index=True)

    return test_data, train_data


def prepare_covtype(dataset_folder, nrows=None):
    df = datasets.fetch_covtype(
        data_home=dataset_folder, as_frame=True)["frame"]
    if nrows is not None:
        df = df[:nrows]
    df = df.astype(np.float32)
    df = df.astype({"Cover_Type": np.int8})
    # Training classifier requires labels [0 1 2 3 4 5 6], but the original data's labels are [1 2 3 4 5 6 7]
    df["Cover_Type"] = df["Cover_Type"] - 1
    return df


def get_connection(pgsqlconfig):
    return psycopg2.connect(
        database=pgsqlconfig["dbname"],
        user=pgsqlconfig["username"],
        password=pgsqlconfig["password"],
        host=pgsqlconfig["host"],
        port=pgsqlconfig["port"]
    )


def make_query(dataset, datasetconfig, column_names):
    # Make query to create table
    if dataset == "epsilon":
        feature_names = '''"row" double precision[]'''
        label_name = "label INTEGER NOT NULL"
        create_query = f"CREATE TABLE ** ({label_name}, {feature_names})"
    elif dataset == "bosch":
        feature_names = ", ".join(
            [f"feature{i} DECIMAL" for i in range(datasetconfig["num_features"])])
        label_name = f"{datasetconfig['y_col']} INTEGER NOT NULL"
        create_query = f"CREATE TABLE ** ({feature_names}, {label_name})"
    elif dataset == "covtype":
        feature_names = ", ".join(
            [f"{col_name} DECIMAL NOT NULL" for col_name in column_names[:-1]])
        label_name = f"{column_names[-1]} INTEGER NOT NULL"
        create_query = f"CREATE TABLE ** ({feature_names}, {label_name})"
    else:
        create_query = datasetconfig["create"]
    train_create_query = create_query.replace(
        "**", f"{datasetconfig['table']}_train", 1)
    test_create_query = create_query.replace(
        "**", f"{datasetconfig['table']}_test", 1)
    return (train_create_query, test_create_query)


def save_as_pickle(train, test, dataset_folder, filename):
    train_pkl_path = relative2abspath(dataset_folder, f"{filename}_train.pkl")
    pickle.dump(train, open(train_pkl_path, "wb"))
    test_csv_path = relative2abspath(dataset_folder, f"{filename}_test.pkl")
    pickle.dump(test, open(test_csv_path, "wb"))
    print(f"{dataset} is saved as train and test pickle files.")


def save_to_csv(train, test, dataset_folder, filename):

    train_csv_path = relative2abspath(dataset_folder, f"{filename}_train.csv")
    train.to_csv(train_csv_path, index=False, header=False)
    test_csv_path = relative2abspath(dataset_folder, f"{filename}_test.csv")
    test.to_csv(test_csv_path, index=False, header=False)
    print(f"{dataset} is saved as train and test CSVs.")
    return (train_csv_path, test_csv_path)


def create_tables(
        connection,
        train_query,
        test_query,
        train_csv_path,
        test_csv_path,
        dataset):

    print("DROPPING TRAIN AND TABLE IF THEY EXIST")
    with connection.cursor() as cursor:
        cursor.execute("DROP TABLE IF EXISTS " +
                       datasetconfig["table"]+"_train")
        connection.commit()

    with connection.cursor() as cursor:
        cursor.execute("DROP TABLE IF EXISTS " +
                       datasetconfig["table"]+"_test")
        connection.commit()

    print("CREATING AND POPULATING TABLES")
    with connection.cursor() as cursor:
        cursor.execute(train_query)
        with open(train_csv_path) as f:
            cursor.copy_expert(
                "COPY "+datasetconfig["table"]+"_train"+" FROM STDIN WITH CSV", f)
        print("LOADED "+datasetconfig["table"]+"_train"+" to DB")
        connection.commit()

    # Create test table
    with connection.cursor() as cursor:
        cursor.execute(test_query)
        with open(test_csv_path) as f:
            cursor.copy_expert(
                "COPY "+datasetconfig["table"]+"_test"+" FROM STDIN WITH CSV", f)
        print("LOADED "+datasetconfig["table"]+"_test"+" to DB")
        connection.commit()

    print("TABLES CREATED AND DATA LOADED")


if __name__ == "__main__":
    # Get settings
    dataset_folder = relative2abspath(dataset_folder)
    if not os.path.exists(dataset_folder):
        os.makedirs(dataset_folder)
    args = parse_arguments()
    dataset, nrows = args.dataset, args.nrows
    config = json.load(open(relative2abspath("config.json")))
    pgsqlconfig, datasetconfig = config["pgsqlconfig"], config[dataset]

    # Download dataset and read as pandas dataframe
    if dataset == 'higgs':
        df = prepare_higgs(dataset_folder, nrows=nrows)
    elif dataset == 'year':
        df = prepare_year(dataset_folder, nrows=nrows)
    elif dataset == 'airline_regression' or dataset == 'airline_classification':
        is_classification = datasetconfig["type"] == "classification"
        df = prepare_airline(is_classification, dataset_folder, nrows=nrows)
    elif dataset == 'epsilon':
        df_test, df_train = prepare_epsilon(nrows=nrows)
    elif dataset == "fraud":
        df = prepare_fraud(dataset_folder, nrows=nrows)
    elif dataset == 'bosch':
        df = prepare_bosch(dataset_folder, nrows=nrows)
    elif dataset == 'covtype':
        df = prepare_covtype(dataset_folder, nrows=nrows)
    else:
        raise ValueError(f"{dataset} not supported")

    # EPSILON FOLLOWS DIFFERENT LOADING INSTRUCTIONS AS
    # IT HAS MORE THAN 1600 COLUMNS
    if dataset == "epsilon":
        train = df_train
        test = df_test
        column_names = list(df_train.columns)
        connection = get_connection(pgsqlconfig)
        print("FETCHING TRAIN AND TEST QUERY EPSILON")
        train_query, test_query = make_query(
            dataset, datasetconfig, column_names)

        print("DROPPING TRAIN AND TABLE IF THEY EXIST")
        with connection.cursor() as cursor:
            cursor.execute("DROP TABLE IF EXISTS " +
                           datasetconfig["table"]+"_train")
            connection.commit()

        with connection.cursor() as cursor:
            cursor.execute("DROP TABLE IF EXISTS " +
                           datasetconfig["table"]+"_test")
            connection.commit()

        print("CREATING TABLES FOR EPSILON")
        with connection.cursor() as cursor:
            cursor.execute(train_query)
            cursor.execute(test_query)
            connection.commit()

        print("LOADING DATA FOR EPSILON")
        columns = [i for i in range(1, 2001)]
        with connection.cursor() as cur:
            train.head()
            rows = len(train)
            for i in range(rows):
                cur.execute("INSERT INTO epsilon_train(label,row) VALUES(%s, %s)", (int(
                    train.loc[i, 0]), list(train.loc[i, columns])))
                if i % 10000 == 0:
                    print(i)

            connection.commit()
            print("LOADED "+datasetconfig["table"]+"_train"+" to DB")

            test.head()
            rows = len(test)
            for i in range(rows):
                cur.execute("INSERT INTO epsilon_test(label,row) VALUES(%s, %s)", (int(
                    test.loc[i, 0]), list(test.loc[i, columns])))
                if i % 10000 == 0:
                    print(i)

            connection.commit()
            print("LOADED "+datasetconfig["table"]+"_test"+" to DB")

        exit()

    # Split dataset
    train_size = math.floor(len(df) * datasetconfig["train"])
    train = df.head(train_size)
    test = df.tail(len(df) - train_size)

    # Store datset
    # ## For wide datasets such as "epsilon", save as pickle file
    # if dataset == "epsilon":
    #     save_as_pickle(train, test, dataset_folder, datasetconfig['filename'])
    #     exit()

    # Store dataset into PostgreSQL database, using copying from CSV strategy
    # First step: save dataframes to csv
    train_csv_path, test_csv_path = save_to_csv(
        train, test, dataset_folder, datasetconfig['filename'])

    # Second step: copy csv to database
    column_names = list(df.columns)
    del df
    gc.collect()
    connection = get_connection(pgsqlconfig)
    print("FETCHING TRAIN AND TEST QUERY")
    train_query, test_query = make_query(dataset, datasetconfig, column_names)
    print("CREATING TRAIN AND TEST TABLES")
    create_tables(connection, train_query, test_query,
                  train_csv_path, test_csv_path, dataset)
