# Onnx with PostgreSQL Comparison Experiments

This repository contains all the code for Random Forest testing on Onnx with PostgreSQL.
In this test, we are using the higgs dataset, which you can download from the following repository.
https://archive.ics.uci.edu/ml/datasets/HIGGS

# Environment
2.6 GHz 6-Core Intel Core i7

25,600 MB Memory

Ubuntu 16.04.7 LTS

TensorFlow 2.6.0

## Install Onnx
```
$pip3 install onnxruntime
```
For more information, please check the following link:
https://onnx.ai/

## Install Postgres
```
$sudo apt update
$sudo apt install postgresql postgresql-contrib
```

# Generate the inputs in PostgreSQL
For testing purpose, we need to first create a table in PostgreSQL and load the input data into the PostgreSQL.

## Create a table and save the inputs to Postgres
We need to process the input. Input needs to be imported into the Postgres database. Please first make sure that there is one database with user names and passwords are both "postgres" in your running environment. We will create tables inside it. You could use the following command to meet the above requirements.
```
$sudo -i -u postgres
$psql
postgres=# \password postgres
Enter new password: <new-password>
postgres=# \q
```

And then we need to create a table named higgs inside this database.
```
postgres=# CREATE TABLE higgs(label REAL NOT NULL, leptonpT REAL NOT NULL, leptoneta REAL NOT NULL, leptonphi REAL NOT NULL, missingenergymagnitude REAL NOT NULL, missingenergyphi REAL NOT NULL, jet1pt REAL NOT NULL, jet1eta REAL NOT NULL, jet1phi REAL NOT NULL, jet1btag REAL NOT NULL, jet2pt REAL NOT NULL, jet2eta REAL NOT NULL, jet2phi REAL NOT NULL, jet2btag REAL NOT NULL, jet3pt REAL NOT NULL, jet3eta REAL NOT NULL, jet3phi REAL NOT NULL, jet3btag REAL NOT NULL, jet4pt REAL NOT NULL, jet4eta REAL NOT NULL, jet4phi REAL NOT NULL, jet4btag REAL NOT NULL, mjj REAL NOT NULL, mjjj REAL NOT NULL, mlv REAL NOT NULL, mjlv REAL NOT NULL, mbb REAL NOT NULL, mwbb REAL NOT NULL, mwwbb REAL NOT NULL);
```
We can check the table information using the following command.
```
postgres=# \d+ higgs
```
After creating the higgs table, we need to load the data to this table. Please use the following command to load the data. Note that you need to change the path of the file located in your machine.
```
postgres=# copy higgs from '/home/jiaqingchen/datasets/higgs/HIGGS.csv' with CSV;
```
After running the above command, it will show that "COPY 11000000". It shows the total number/size of the testing input.

## Run the test program
After the testing input is successfully loaded into the PostgreSQL, it is ready to run the testing in Onnx.
```
$python3 Onnx_Test.py
```
