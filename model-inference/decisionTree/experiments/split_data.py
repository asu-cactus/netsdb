import pandas as pd
import sys
import json
import psycopg2
from sklearn.model_selection import train_test_split

args = sys.argv
DATASET = "higgs"

config = json.load(open("config.json"))
datasetconfig = config[DATASET]
pgsqlconfig = config["pgsqlconfig"]
test = float(datasetconfig["test"])
table_name = datasetconfig["table"]
header = datasetconfig["header"]

if len(args) > 1:
    DATASET = args[-1]

df = None
if header:
    df = pd.read_csv(datasetconfig["filename"])
else:
    df = pd.read_csv(datasetconfig["filename"],header=None)

print("LOADED",datasetconfig["filename"])

train, test = train_test_split(df, test_size=test)

print(len(df))
print(len(train))
print(len(test))

train.to_csv(datasetconfig["filename"]+"_train.csv",index=False,header=False)
test.to_csv(datasetconfig["filename"]+"_test.csv",index=False,header=False)

print("SAVED",datasetconfig["filename"]+"_train.csv",datasetconfig["filename"]+"_test.csv")

connection = psycopg2.connect(
    database = pgsqlconfig["dbname"],
    user = pgsqlconfig["username"],
    password = pgsqlconfig["password"],
    host = pgsqlconfig["host"],
    port = pgsqlconfig["port"]
)

create_query = datasetconfig["create"]
create_query = create_query.split("**")

with connection.cursor() as cursor:
    cursor.execute(create_query[0]+datasetconfig["table"]+"_train"+create_query[1])
    with open(datasetconfig["filename"]+"_train.csv") as f:
        cursor.copy_expert("COPY "+datasetconfig["table"]+"_train"+" FROM STDIN WITH CSV", f)
    print("LOADED "+datasetconfig["table"]+"_train"+" to DB")
    connection.commit()

with connection.cursor() as cursor:
    cursor.execute(create_query[0]+datasetconfig["table"]+"_test"+create_query[1])
    with open(datasetconfig["filename"]+"_test.csv") as f:
        cursor.copy_expert("COPY "+datasetconfig["table"]+"_test"+" FROM STDIN WITH CSV", f)
    print("LOADED "+datasetconfig["table"]+"_test"+" to DB")
    connection.commit()