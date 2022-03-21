# PostgreSQL Comparison Experiments

This repository contains all the code for Random Forest testing on PostgreSQL.
In this test, we are using the higgs dataset, which you can download from the following repository.
https://archive.ics.uci.edu/ml/datasets/HIGGS

# Environment
AWS r4xlarge 30.5GB

Ubuntu 18.04

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

## Create several tables and save the random forest of 10 trees to PostgreSQL
We need to create 10 tables for 10 trees and one table for the random forest.
We aslo need one more table to save the feature names of the inputs table.
```
postgres=# CREATE TABLE tree0(id INTEGER NOT NULL, index INTEGER NOT NULL, value REAL NOT NULL, isleaf boolean NOT NULL, leftchild INTEGER NOT NULL, rightchild INTEGER NOT NULL);
postgres=# CREATE TABLE tree1(id INTEGER NOT NULL, index INTEGER NOT NULL, value REAL NOT NULL, isleaf boolean NOT NULL, leftchild INTEGER NOT NULL, rightchild INTEGER NOT NULL);
postgres=# CREATE TABLE tree2(id INTEGER NOT NULL, index INTEGER NOT NULL, value REAL NOT NULL, isleaf boolean NOT NULL, leftchild INTEGER NOT NULL, rightchild INTEGER NOT NULL);
postgres=# CREATE TABLE tree3(id INTEGER NOT NULL, index INTEGER NOT NULL, value REAL NOT NULL, isleaf boolean NOT NULL, leftchild INTEGER NOT NULL, rightchild INTEGER NOT NULL);
postgres=# CREATE TABLE tree4(id INTEGER NOT NULL, index INTEGER NOT NULL, value REAL NOT NULL, isleaf boolean NOT NULL, leftchild INTEGER NOT NULL, rightchild INTEGER NOT NULL);
postgres=# CREATE TABLE tree5(id INTEGER NOT NULL, index INTEGER NOT NULL, value REAL NOT NULL, isleaf boolean NOT NULL, leftchild INTEGER NOT NULL, rightchild INTEGER NOT NULL);
postgres=# CREATE TABLE tree6(id INTEGER NOT NULL, index INTEGER NOT NULL, value REAL NOT NULL, isleaf boolean NOT NULL, leftchild INTEGER NOT NULL, rightchild INTEGER NOT NULL);
postgres=# CREATE TABLE tree7(id INTEGER NOT NULL, index INTEGER NOT NULL, value REAL NOT NULL, isleaf boolean NOT NULL, leftchild INTEGER NOT NULL, rightchild INTEGER NOT NULL);
postgres=# CREATE TABLE tree8(id INTEGER NOT NULL, index INTEGER NOT NULL, value REAL NOT NULL, isleaf boolean NOT NULL, leftchild INTEGER NOT NULL, rightchild INTEGER NOT NULL);
postgres=# CREATE TABLE tree9(id INTEGER NOT NULL, index INTEGER NOT NULL, value REAL NOT NULL, isleaf boolean NOT NULL, leftchild INTEGER NOT NULL, rightchild INTEGER NOT NULL);
postgres=# CREATE TABLE rf(rf_id INTEGER NOT NULL, tree_id INTEGER NOT NULL);
postgres=# CREATE TABLE columns(column_name text NOT NULL, column_id INTEGER NOT NULL);
```

And then we need to load the data to these tables. Please use the following command to load the data. Note that you need to change the path of the file located in your machine.
```
postgres=# copy tree0 from '/home/ubuntu/netsdb/src/decisionTree/experiments/higgs/PostgreSQL/rf_tables/tree_0.csv' with CSV;
postgres=# copy tree1 from '/home/ubuntu/netsdb/src/decisionTree/experiments/higgs/PostgreSQL/rf_tables/tree_1.csv' with CSV;
postgres=# copy tree2 from '/home/ubuntu/netsdb/src/decisionTree/experiments/higgs/PostgreSQL/rf_tables/tree_2.csv' with CSV;
postgres=# copy tree3 from '/home/ubuntu/netsdb/src/decisionTree/experiments/higgs/PostgreSQL/rf_tables/tree_3.csv' with CSV;
postgres=# copy tree4 from '/home/ubuntu/netsdb/src/decisionTree/experiments/higgs/PostgreSQL/rf_tables/tree_4.csv' with CSV;
postgres=# copy tree5 from '/home/ubuntu/netsdb/src/decisionTree/experiments/higgs/PostgreSQL/rf_tables/tree_5.csv' with CSV;
postgres=# copy tree6 from '/home/ubuntu/netsdb/src/decisionTree/experiments/higgs/PostgreSQL/rf_tables/tree_6.csv' with CSV;
postgres=# copy tree7 from '/home/ubuntu/netsdb/src/decisionTree/experiments/higgs/PostgreSQL/rf_tables/tree_7.csv' with CSV;
postgres=# copy tree8 from '/home/ubuntu/netsdb/src/decisionTree/experiments/higgs/PostgreSQL/rf_tables/tree_8.csv' with CSV;
postgres=# copy tree9 from '/home/ubuntu/netsdb/src/decisionTree/experiments/higgs/PostgreSQL/rf_tables/tree_9.csv' with CSV;
postgres=# copy rf from '/home/ubuntu/netsdb/src/decisionTree/experiments/higgs/PostgreSQL/rf_tables/rf.csv' with CSV;
postgres=# copy columns from '/home/ubuntu/netsdb/src/decisionTree/experiments/higgs/PostgreSQL/rf_tables/columns.csv' with CSV;
```
After running the above commands, it will show that "COPY XXX". It shows the total nodes of each tree or total trees in the random forest. If you run the above commands in order, the result should be as follows.
COPY 511
COPY 509
COPY 511
COPY 511
COPY 491
COPY 509
COPY 509
COPY 511
COPY 511
COPY 509
COPY 10
COPY 29

Next step, we need to create the UDF model logic function of tree inference:
```
postgres=# \i /home/ubuntu/netsdb/src/decisionTree/experiments/higgs/PostgreSQL/code/genericfun.sql
```

We also need to create the UDF query function of the above UDF model logic function.
We can change the batch size in this function.
```
postgres=# \i /home/ubuntu/netsdb/src/decisionTree/experiments/higgs/PostgreSQL/code/callgenericfun.sql
```

At this step, we can test this UDF query function using the following command:
```
postgres=# select call_generic();
```

## Run the test program
Use the following command to test this UDF query function execution time in PostgreSQL.
```
postgres=# \i /home/ubuntu/netsdb/src/decisionTree/experiments/higgs/PostgreSQL/code/runtime4generic.sql
```
