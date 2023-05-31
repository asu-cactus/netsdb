# CNN Benching Marking Using Pytorch and Tensorflow
This module implements Conv2d operation using Pytorch and Tensorflow.

## Dependencies
- Ubuntu 
- Pytorch
- Tensorflow 
- Python
- PostgreSQL
  - Create user: `postgres`
  - With password: `postgres`

## Run

### Tensorflow
`$ python3 cnn-tensorflow.py`

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

#### Example
`$ python3 cnn-tensorflow.py --input '40, 224, 224, 64' --kernel '3,3,64,64' --stride 1 --images 10 --loadFromFile Y`

Above example, performs 10 convolution operation on input (N: 40, H: 224, W: 224, C: 64) with kernel (H: 3, W: 3, C: 64, number of filters: 64) producing 10 output of shape (40, 222, 222, 64) 

### Pytorch
$ python3 cnn-pytorch.py

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

#### Example
`$ python3 cnn-pytorch.py --input '40, 64, 224, 224' --kernel '64,64,3,3' --images 10 --stride 1 --loadFromFile Y`

Above example, performs 10 convolution operation on input (N: 40, C: 64, H: 224, W: 64) with kernel (number of filters: 64, C: 3, H: 64, W: 64) producing 10 output of shape (N: 40, C: 64, H: 222, W: 222)
