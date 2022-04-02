# Convolutional Neural Network

We implemented several convolutional neural network implementation:

(1) We encapsulate the conv2D logic in a transformation UDF
[Convolutional Neural Network Application Code based on UDF-encapsulation](https://github.com/asu-cactus/netsdb/blob/master/src/tests/source/Conv2dProjTest.cc)

An image is in the shape of 112x112x3
The convolutional neural network consists of 64 7x7x3 filters.


(2) We run conv2D as a matrix multiplication based on spatial rewriting
[Convolutional Neural Network Application Code based on spatial-rewriting](https://github.com/asu-cactus/netsdb/blob/master/src/tests/source/PipelinedConv2dMemFuseTest.cc)

An image is in the shape of 112x112x3
The convolutional neural network consists of 64 7x7x3 filters

## Pytorch Dependency Installation

- Tested configuration on Ubuntu 20.04 LTS https://pytorch.org/get-started/locally/
    - PyTorch Build: Stable (1.10.2)
    - OS: Linux
    - Package: Source
    - Language: Python
    - Compute platform: CPU
- Install Anaconda https://docs.anaconda.com/anaconda/install/
- Pytorch installation using source https://github.com/pytorch/pytorch#from-source

- In [SContruct](https://github.com/asu-cactus/netsdb/blob/master/SConstruct)
    - Update `LIBPYTORCH_PATH` to your installation path 
    - Update `LIBPYTORCH_ROOT` to your installation path
- Last step is to set LD_LIBRARY_PATH
  ```
  export LD_LIBRARY_PATH="/home/ubuntu/pytorch/torch/lib:/home/ubuntu/anaconda3/lib:$LD_LIBRARY_PATH"
  ```
  Replace: `/home/ubuntu/pytorch/torch/lib` and `/home/ubuntu/anaconda3/lib` with your install paths.
  
  Note: Without this step you might run into `libtorch.so` library missing while starting netsdb cluster.


## Compilation:

### The version based on UDF-encapsulation 
```
scons libConv2DProjTest
```

### The version based on spatial rewriting
```
scons libConv2DMemFuseTest
```

## Run:

Once you start the netsDB system [Start Cluster](https://github.com/asu-cactus/netsdb/blob/master/README.md), run following command:

### The version based on UDF-encapsulation 

```
bin/Conv2dProjTest whetherToLoadData whetherToCreateSet numImages conv2D-mode(aten-conv2d or eigen-spatial) inputDimension kernelDimension stride
```

You can pass the command line parameters or modify variables in the example application source to control the block shape and the size of the word2vec layer.

| Parameter/Variables       | Explanation                                                          |
| ------------------------- |----------------------------------------------------------------------|
| whetherToLoadData         | whether to load input or reuse input: Y for yes, N for no            |
| whetherToCreateSet        | whether to create a new set to store input data: Y for yes, N for no |
| numImages                 | number of input images to generate for inferences                    |
| conv2D-mode               | Run Aten convolution, eigen-spatial                                  |
| inputDimension            | Default: "1,64,112,112", format: n, c, w, h                          |
| kernelDimension           | Default: "64,64,1,1", format: nk, ck, wk, hk                         |
| stride                    | Default: 1                                                           |

#### Examples
```
bin/Conv2dProjTest Y Y 5 "aten-conv2d" "1,64,112,112" "64,64,1,1"
bin/Conv2dProjTest N N 5 "aten-conv2d" "1,64,112,112" "64,64,1,1"
```

### The version based on spatial rewriting

```
bin/PipelinedConv2dMemFuseTest whetherToLoadData blockDimensionY whetherToMaterializeModel
```

An image is in the shape of 112x112x3
The convolutional neural network consists of 64 7x7x3 filters.
For this experiment, the input images and filters should be loaded from .np files.

For example:

```
conv2d_memory_fusion::load_imgs_from_file<conv2d_memory_fusion::Image>(
            pdbClient, "/home/ubuntu/images_100_3_112_112.np", dbName, imageset, 100, 3, 112, 112);
conv2d_memory_fusion::load_imgs_from_file<conv2d_memory_fusion::Kernel>(
            pdbClient, "/home/ubuntu/conv2d/kernel_64_3_7_7.np", dbName, kernelset, 64, 3, 7, 7);
```

The file paths can be customized [here](https://github.com/asu-cactus/netsdb/blob/master/src/tests/source/PipelinedConv2dMemFuseTest.cc)


| Parameter/Variables       | Explanation                                               |
| ------------------------- |-----------------------------------------------------------|
| whetherToLoadData         | Whether to load input or reuse input: Y for yes, N for no |
| whetherToMaterializeModel | For performance optimization: Y for yes, N for no         |


