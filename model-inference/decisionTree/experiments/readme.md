# Decision Tree Experiments

## Project Setup

### PostgreSQL

```
sudo apt update
sudo apt install wget gzip bzip2 unzip xz-utils
wget https://archive.ics.uci.edu/ml/machine-learning-databases/00280/HIGGS.csv.gz
gzip -d HIGGS.csv.gz
sudo apt install postgresql postgresql-contrib
sudo -i -u postgres
psql
postgres=# \password postgres
Enter new password: <new-password>
postgres=# \q
postgres=# CREATE TABLE higgs(label REAL NOT NULL, leptonpT REAL NOT NULL, leptoneta REAL NOT NULL, leptonphi REAL NOT NULL, missingenergymagnitude REAL NOT NULL, missingenergyphi REAL NOT NULL, jet1pt REAL NOT NULL, jet1eta REAL NOT NULL, jet1phi REAL NOT NULL, jet1btag REAL NOT NULL, jet2pt REAL NOT NULL, jet2eta REAL NOT NULL, jet2phi REAL NOT NULL, jet2btag REAL NOT NULL, jet3pt REAL NOT NULL, jet3eta REAL NOT NULL, jet3phi REAL NOT NULL, jet3btag REAL NOT NULL, jet4pt REAL NOT NULL, jet4eta REAL NOT NULL, jet4phi REAL NOT NULL, jet4btag REAL NOT NULL, mjj REAL NOT NULL, mjjj REAL NOT NULL, mlv REAL NOT NULL, mjlv REAL NOT NULL, mbb REAL NOT NULL, mwbb REAL NOT NULL, mwwbb REAL NOT NULL);

postgres=# copy higgs from 'HIGGS.csv' with CSV;
postgres=# copy higgs from 'HIGGS.csv' with CSV;
```

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
python train_model.py dataset classifier

python test_model.py dataset classifier framework batch_size [gpu]


**Examples**

python train_model.py higgs randomforest

python train_model.py higgs xgboost

python test_model.py higgs randomforest Sklearn 100000

```

Get CPU Usage

```
echo "CPU Usage: "$[100-$(vmstat 1 2|tail -1|awk '{print $15}')]"%"
```
