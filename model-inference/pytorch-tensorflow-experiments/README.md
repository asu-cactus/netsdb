# Experiments for the Deep Learning Centric Architecture

## Transformer Benchmarking using Pytorch and TensorFlow

Run inference on question-answering model using huggingface with loaded data from postgres by using connectorX


### Dependencies

- Ubuntu
- Pytorch
- Python
- PostgreSQL
  - Create user: `postgres`
  - With password: `postgres`
- ConnectorX
- HuggingFace

### Models:
- deepset/tinyroberta-squad2 (81.5M parameters, 326 MB)
- deepset/roberta-base-squad2 (124M parameters, 496 MB)
- deepset/bert-large-uncased-whole-word-masking-squad2 (335M parameters, 1372 MB)

### Dataset:

Question Answering Dataset - SQuAD 2.0 [Link](https://rajpurkar.github.io/SQuAD-explorer/)

### Run:

First, store the SQuAD dataset into Postgres into three columns: index, question, context

Then:

```bash
python huggingface_lm_connectorX.py
```

## FFNN Benchmarking using Pytorch and TensorFlow
This module implements FFNN operation using Pytorch and TensorFlow.

### Dependencies
- Ubuntu
- Pytorch
- Tensorflow
- Python
- PostgreSQL
  - Create user: `postgres`
  - With password: `postgres`
- ConnectorX

### Run

#### Tensorflow

##### Store the dataset into Postgres:

For amazon_14K classification:

```bash
cd FFNN_MLP/tensorflow
python amazon_14K_model.py
```

For keras-bert-FC classification:

```bash
cd FFNN_MLP/tensorflow
python keras_bert_fc_model.py
```
##### Run the inferences using ConnectorX

For amazon_14K classification:

```bash
python amazon_14K_inference_connectorX.py
```

For keras-bert-FC classification:

```bash
python keras_bert_fc_inference_connectorX.py
```

#### Pytorch

##### Store the dataset into Postgres:

For amazon_14K classification:

```bash
cd FFNN_MLP/pytorch
python amazon_14K_model.py
```

For keras-bert-FC classification:

```bash
cd FFNN_MLP/pytorch
python keras_bert_fc_model.py
```
##### Run the inferences using ConnectorX

For amazon_14K classification:

```bash
python amazon_14K_inference_connectorX.py
```

For keras-bert-FC classification:

```bash
python keras_bert_fc_inference_connectorX.py
```


## CNN Benching Marking Using Pytorch and Tensorflow
This module implements Conv2d operation using Pytorch and TensorFlow.

### Dependencies
- Ubuntu 
- Pytorch
- Tensorflow 
- Python
- PostgreSQL
  - Create user: `postgres`
  - With password: `postgres`

### Run

#### Tensorflow

`$ cd CNN-conv2d`

Load the data

`$ python3 load-data.py`


Run the inferences

`$ python3 cnn-connectorx-tensorflow.py`

This script accepts following parameters

<pre>
optional arguments:
  -h, --help                    show this help message and exit
  --images IMAGES               total number of input images for inference
  --inputsize INPUTSIZE         input size 'batchsize, channels, width, height', default value = '10, 224, 224, 3'
  --kernelsize KERNELSIZE       kernel size 'number of kernels, channels, width, height', default value = '7, 7, 3, 64'
  --stride STRIDE               stride along width, height', default value = '1'
  --loadFromFile LOADFROMFILE   load data from file Y: Yes, N: No', default value = N'
</pre>

##### Example
`$ python3 cnn-connectorx-tensorflow.py --input '40, 224, 224, 64' --kernel '3,3,64,64' --stride 1 --images 10 --loadFromFile Y`

Above example, performs 10 convolution operation on input (N: 40, H: 224, W: 224, C: 64) with kernel (H: 3, W: 3, C: 64, number of filters: 64) producing 10 output of shape (40, 222, 222, 64) 

#### Pytorch

`$ cd CNN-conv2d`

Load the data

`$ python3 load-data.py`

Run the inferences

`$ python3 cnn-pytorch.py`

This script accepts following parameters

<pre>
optional arguments:
  -h, --help                        show this help message and exit
  --images IMAGES                   number of images for inferences
  --inputsize INPUTSIZE             input size 'batchsize, channels, width, height', default value = '10, 3, 224, 224'
  --kernelsize KERNELSIZE           kernel size 'number of kernels, channels, width, height', default value = '64, 3, 7, 7'
  --stride STRIDE                   stride along width, height', default value = '1'
  --loadFromFile LOADFROMFILE       load data from file Y: Yes, N: No', default value = N'
</pre>

##### Example
`$ python3 cnn-connectorx-pytorch.py --input '40, 64, 224, 224' --kernel '64,64,3,3' --images 10 --stride 1 --loadFromFile Y`

Above example, performs 10 convolution operation on input (N: 40, C: 64, H: 224, W: 64) with kernel (number of filters: 64, C: 3, H: 64, W: 64) producing 10 output of shape (N: 40, C: 64, H: 222, W: 222)
