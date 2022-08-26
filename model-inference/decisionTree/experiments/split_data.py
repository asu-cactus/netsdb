from itertools import count
from pyexpat import features
import pandas as pd
import sys
import json
import psycopg2
from sklearn.model_selection import train_test_split
from model_helper import relative2abspath
from data_processing import dataset_folder

DATASET = "airline_classification"
args = sys.argv
if len(args) > 1:
    DATASET = args[-1]

config = json.load(open(relative2abspath("config.json")))
datasetconfig = config[DATASET]
pgsqlconfig = config["pgsqlconfig"]
test_size = float(datasetconfig["test"])
table_name = datasetconfig["table"]
header = datasetconfig["header"]

df = None
if header:
    df = pd.read_csv(relative2abspath(dataset_folder, datasetconfig["filename"]), header=0)
else:
    df = pd.read_csv(relative2abspath(dataset_folder, datasetconfig["filename"]), header=None)

print("LOADED",datasetconfig["filename"])

train, test = train_test_split(df, random_state=77, test_size=test_size)

print(len(df))
print(len(train))
print(len(test))

train_csv_path = relative2abspath(dataset_folder, f"{datasetconfig['filename']}_train.csv")
train.to_csv(train_csv_path,index=False,header=False)
test_csv_path = relative2abspath(dataset_folder, f"{datasetconfig['filename']}_test.csv")
test.to_csv(test_csv_path,index=False,header=False)

print("SAVED",datasetconfig["filename"]+"_train.csv",datasetconfig["filename"]+"_test.csv")

connection = psycopg2.connect(
    database = pgsqlconfig["dbname"],
    user = pgsqlconfig["username"],
    password = pgsqlconfig["password"],
    host = pgsqlconfig["host"],
    port = pgsqlconfig["port"]
)

# Make query to create table
if DATASET == "epsilon":
    feature_names = ", ".join([f"F{i} DECIMAL NOT NULL" for i in range(datasetconfig["num_features"])])
    label_name = "Label INTEGER NOT NULL"
    create_query = f"CREATE TABLE ** ({label_name}, {feature_names})"
else:
    create_query = datasetconfig["create"]
train_create_query = create_query.replace("**", f"{datasetconfig['table']}_train", 1)
test_create_query = create_query.replace("**", f"{datasetconfig['table']}_test", 1)

# Create train table
with connection.cursor() as cursor:
    cursor.execute("DROP TABLE IF EXISTS " + datasetconfig["table"]+"_train")
    connection.commit()

with connection.cursor() as cursor:
    cursor.execute(train_create_query)
    with open(train_csv_path) as f:
        cursor.copy_expert("COPY "+datasetconfig["table"]+"_train"+" FROM STDIN WITH CSV", f)
    print("LOADED "+datasetconfig["table"]+"_train"+" to DB")
    connection.commit()

# Create test table
with connection.cursor() as cursor:
    cursor.execute("DROP TABLE IF EXISTS " + datasetconfig["table"]+"_test" )
    connection.commit()

with connection.cursor() as cursor:
    cursor.execute(test_create_query)
    with open(test_csv_path) as f:
        cursor.copy_expert("COPY "+datasetconfig["table"]+"_test"+" FROM STDIN WITH CSV", f)
    print("LOADED "+datasetconfig["table"]+"_test"+" to DB")
    connection.commit()
