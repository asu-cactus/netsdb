# Decision Tree Experiments

## Project Setup

### PostgreSQL

```
sudo apt update
sudo apt install wget gzip
wget https://archive.ics.uci.edu/ml/machine-learning-databases/00280/HIGGS.csv.gz
gzip
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
