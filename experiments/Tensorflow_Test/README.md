# Introduction

This repository contains all the code for text classification and extreme classification testing on Tensorflow.

# Environment
AWS r4.xlarge for CPU testing and AWS g4dn.2xlarge for GPU testing.

AMI Deep Learning Base AMI (Ubuntu 18.04) Version 45.0.

TensorFlow 2.6.2

## Install TensorFlow Hub
```
$pip3 install --upgrade tensorflow-hub
```

## Install Postgres
```
$sudo apt update
$sudo apt install postgresql postgresql-contrib
```

# Text Classification
For text classification, please go to Text_Classification_Test folder.

## Download the models
Please download the model when running for the first time. The model is saved locally in H5 format. Please run:
```
$python3 Download_Models.py
```

## Change the weights(Optional)
At this time, the weights of the model downloaded to the local is float32. Our tests for text classification based on look up function are using float32 precision and the tests based on matrix multiplication are using double precision. We will load one of the previous models and then change the wights from float32 to double while testing in matrix multiplication. However, if you would like to save the weights of the models as double precision, please run Change_Weights_Float_to_Double.py: (This is optional. Not do so will not affect the testing results.)
```
$python3 Change_Weights_Float_to_Double.py
```

## Generate the inputs and save the inputs to CSV and Postgres
After processing the model, we need to process the input. Input needs to be saved as CSV or imported into the Postgres database. Please first make sure that there is one database with user names and passwords are both "postgres" in your running environment. We will create tables inside it. You could use the following command to meet the above requirements.
```
$sudo -i -u postgres
$psql
postgres=# \password postgres
Enter new password: <new-password>
postgres=# \q
```

And then run the following python script to save the input to CSV file. This script will generate two CSV files. One is named input_float.csv, which will be used for look up function testing and the other is named input_double.csv, which will be used for matrix multiplication testing.
```
$python3 Save_Input_to_CSV.py
```

For the word2vec-inference-MM-exp.py, which is using matrix multiplication in double precision, we have 2 methods saving in Postgres. One is saving all the features in one column and the other is saving in multiple columns. We have to run the following 2 python scripts to save the input to Postgres.
```
$python3 Load_Data_to_Postgres.py
$python3 Load_Postgres_M_Col_Text.py
```

For the word2vec-inference-exp.py, which is using look up function in float32 precision, we only have one method saving in Postgres, which is saving all the features in one column. We have to run the following python script to save the input to Postgres.
```
$python3 Load_Data_to_Postgres_G.py
```

## Run the test program
There are 2 kinds of methods are testing for text classification. Our tests for text classification based on the look up function are using float32 precision and the tests based on matrix multiplication are using double precision. word2vec-inference-MM-exp.py is using matrix multiplication and word2vec-inference-exp.py is using the look up function.

For the word2vec-inference-MM-exp.py, which is using matrix multiplication in double precision, you can set the number of models you would like to test while running the python script. The default number of models is 1. The following is an example for running the tests for 2 models.
```
$python3 word2vec-inference-MM-exp.py 2
```

For the word2vec-inference-exp.py, which is using look up function in float32 precision, you can also set the number of models you would like to test while running the python script. The default number of models is 1. The following is an example for running the tests for 2 models.
```
$python3 word2vec-inference-exp.py 2
```

# Extreme Classification
For extreme classification, please go to Extreme_Classification_Test folder. This code can run on both CPU and GPU environment as mentioned above. On r4.xlarge, it will run on CPU and on g4dn.2xlarge, it will run on GPU.

## Create the model
Please create the model when running for the first time. The model is saved locally in H5 format. Please run:
```
$python3 Build_Model.py
```

You could save the weights of the model either in float32 or double. Our tests are under double precision, so the default setting is double precision.

## Generate the inputs and save the inputs to CSV and Postgres
After processing the model, we need to process the input. Input needs to be saved as CSV or imported into the Postgres database. Please first make sure that there is one database with user names and passwords are both "postgres" in your running environment. We will create tables inside it. You could use the following command to meet the above requirements.
```
$sudo -i -u postgres
$psql
postgres=# \password postgres
Enter new password: <new-password>
postgres=# \q
```

And then run the following three python scripts in turn.
```
$python3 Load_Data_to_Postgres.py
$python3 Load_Postgres_M_Col.py
$python3 Save_Input_to_CSV.py
```

## Run the test program
The only one testing method for text classification, which is based on matrix multiplication is using double precision. Set the number of models you would like to test while running extreme_classification_model_exp.py. The default number of models is 1. The following is an example for running the tests for 2 models.
```
$python3 extreme_classification_model_exp.py 2
```