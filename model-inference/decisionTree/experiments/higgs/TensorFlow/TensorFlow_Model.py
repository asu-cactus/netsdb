import tensorflow_decision_forests as tfdf
import pandas as pd
import numpy as np
import time
import psycopg2
import connectorx as cx
import tensorflow as tf

# Load the dataset in a Pandas dataframe.
colnames=['label','leptonpt','leptoneta','leptonphi','missingenergymagnitude','missingenergyphi','jet1pt','jet1eta','jet1phi','jet1btag','jet2pt','jet2eta','jet2phi','jet2btag','jet3pt','jet3eta','jet3phi','jet3btag','jet4pt','jet4eta','jet4phi','jet4btag','mjj','mjjj','mlv','mjlv','mbb','mwbb','mwwbb']
"""
for i in range(28):
    colnames.append('F'+str(i))
"""

df = pd.read_csv("HIGGS.csv",header=None,names=colnames)
#print(df.head(3))

label = 'label'
classes = df[label].unique().tolist()
print(f"Label classes: {classes}")
df[label] = df[label].map(classes.index)

# Split the dataset into a training and a testing dataset.
def split_dataset(dataset, test_ratio=0.30):
  #Splits a panda dataframe in two.
  test_indices = np.random.rand(len(dataset)) < test_ratio
  return dataset[~test_indices], dataset[test_indices]

train_ds_pd, test_ds_pd = split_dataset(df)
print("{} examples in training, {} examples for testing.".format(len(train_ds_pd), len(test_ds_pd)))

# Convert the dataset into a TensorFlow dataset.
ds = tfdf.keras.pd_dataframe_to_tf_dataset(df, label=label)
train_ds = tfdf.keras.pd_dataframe_to_tf_dataset(train_ds_pd, label=label)
test_ds = tfdf.keras.pd_dataframe_to_tf_dataset(test_ds_pd, label=label)

# Train the model
model = tfdf.keras.RandomForestModel(num_trees=10,max_depth=8)
model.fit(train_ds)

# Look at the model.
model.summary()

# Export to a TensorFlow SavedModel.
# Note: the model is compatible with Yggdrasil Decision Forests.
model.save("rf-model-higgs")

"""
exe_total_time = 0
total_time_start = time.time()
try:
    query = "SELECT leptonpT,leptoneta,leptonphi,missingenergymagnitude,missingenergyphi,"+"jet1pt,jet1eta,jet1phi,jet1btag,jet2pt,jet2eta,jet2phi,jet2btag,jet3pt,jet3eta,"+"jet3phi,jet3btag,jet4pt,jet4eta,jet4phi,jet4btag,mjj,mjjj,mlv,mjlv,mbb,mwbb,mwwbb from higgs"
    #query = "SELECT * from higgs"
    data = cx.read_sql("postgresql://postgres:postgres@localhost:5432/postgres", query)
    ds = tfdf.keras.pd_dataframe_to_tf_dataset(data)
    time_start = time.time()
    pred = model.predict(ds)
    time_end=time.time()
    exe_total_time = exe_total_time + (time_end-time_start)
except psycopg2.Error as e:
    t_message = "Postgres Database error: " + e + "/n"
total_time_end=time.time()

print('exe time cost',exe_total_time*1000,'ms')
print('total time cost',(total_time_end-total_time_start)*1000,'ms')
"""