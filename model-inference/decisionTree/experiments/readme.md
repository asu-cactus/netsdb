# Decision Tree Experiments

## Project Setup

### PostgreSQL

```
#Install tools to download and unzip dataset.
sudo apt update
sudo apt install wget gzip bzip2 unzip xz-utils
wget https://archive.ics.uci.edu/ml/machine-learning-databases/00280/HIGGS.csv.gz
gzip -d HIGGS.csv.gz

#Install postgres DBMS
sudo apt install postgresql postgresql-contrib

##The following steps are optional. They show you how to login and create a table/relation. 
##The python script split_data.py will create the
#Login in
sudo -i -u postgres

#Run postgres DBMS
psql

#Default username and password are both "postgres", which are used in our config.json file. 
#In case you want to change the password, you can do the following:
postgres=# \password postgres
Enter new password: <new-password>

#Create Table and load 
postgres=# CREATE TABLE higgs(label REAL NOT NULL, leptonpT REAL NOT NULL, leptoneta REAL NOT NULL, leptonphi REAL NOT NULL, missingenergymagnitude REAL NOT NULL, missingenergyphi REAL NOT NULL, jet1pt REAL NOT NULL, jet1eta REAL NOT NULL, jet1phi REAL NOT NULL, jet1btag REAL NOT NULL, jet2pt REAL NOT NULL, jet2eta REAL NOT NULL, jet2phi REAL NOT NULL, jet2btag REAL NOT NULL, jet3pt REAL NOT NULL, jet3eta REAL NOT NULL, jet3phi REAL NOT NULL, jet3btag REAL NOT NULL, jet4pt REAL NOT NULL, jet4eta REAL NOT NULL, jet4phi REAL NOT NULL, jet4btag REAL NOT NULL, mjj REAL NOT NULL, mjjj REAL NOT NULL, mlv REAL NOT NULL, mjlv REAL NOT NULL, mbb REAL NOT NULL, mwbb REAL NOT NULL, mwwbb REAL NOT NULL);

postgres=# copy higgs from 'HIGGS.csv' with CSV;

#Quite postgres DBMS
postgres=# \q
```
You must first run split_data.py to split a dataset into training part and testing part, and load both parts to PostgreSQL, which is a prerequisite for running train_model.py and test_model.py.


TO MOUNT THE DRIVE ON EC2

```
sudo file -s /dev/nvme1n1
sudo mkfs -t xfs /dev/nvme1n1
sudo mkdir /mnt/data
sudo mount /dev/nvme1n1 /mnt/data
cd /mnt
sudo chmod 777 data
```

REQUIRED LIBRARIES

```
conda activate python3
pip install connectorx
pip install xgboost
pip install hummingbird_ml[extra]
pip install skl2onnx
pip install onnxmltools
pip install onnxruntime
```

DATASETS

```
#higgs
wget https://archive.ics.uci.edu/ml/machine-learning-databases/00280/HIGGS.csv.gz
gzip -d HIGGS.csv.gz

#airline
wget "http://kt.ijs.si/elena_ikonomovska/datasets/airline/airline_14col.data.bz2"
bzip2 -dk airline_14col.data.bz2

#fraud
kaggle datasets download mlg-ulb/creditcardfraud -f creditcard.csv.zip
unzip creditcard.csv.zip

#year
wget "https://archive.ics.uci.edu/ml/machine-learning-databases/00203/YearPredictionMSD.txt.zip"
unzip YearPredictionMSD.txt.zip

#epsilon
wget https://www.csie.ntu.edu.tw/~cjlin/libsvmtools/datasets/binary/epsilon_normalized.xz
wget https://www.csie.ntu.edu.tw/~cjlin/libsvmtools/datasets/binary/epsilon_normalized.t.xz
unxz epsilon_normalized.xz
unxz epsilon_normalized.t.xz
```

To run a certain experiment

Datasets: higgs

Classifiers: xgboost, randomforest

Frameworks: Sklearn, ONNXCPU, TreeLite, HummingbirdPytorchCPU, HummingbirdTorchScriptCPU, HummingbirdTVMCPU

```
python split_data.py dataset

python train_model.py -d dataset -c classifier

python convert_trained_model_to_framework.py -d dataset -c classifier -f framework

python test_model.py dataset classifier framework batch_size [gpu]


**Examples**

python split_data.py higgs

python train_model.py -d higgs -c randomforest
python train_model.py -d higgs -c xgboost

python convert_trained_model_to_framework.py -d higgs -c randomforest -f pytorch,torch,tf-df,onnx
python convert_trained_model_to_framework.py -d higgs -c xgboost -f pytorch,torch,onnx,treelite

python test_model.py higgs randomforest Sklearn 100000

```

Get CPU Usage

```
echo "CPU Usage: "$[100-$(vmstat 1 2|tail -1|awk '{print $15}')]"%"
```
