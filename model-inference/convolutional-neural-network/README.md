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
bin/Conv2dProjTest whetherToLoadData whetherToCreateSet numImages conv2D-mode(aten-conv2d or eigen-spatial)
```

An image is in the shape of 112x112x3
The convolutional neural network consists of 64 7x7x3 filters.

You can pass the command line parameters or modify variables in the example application source to control the block shape and the size of the word2vec layer.

| Parameter/Variables       | Explanation                                                          |
| ------------------------- |----------------------------------------------------------------------|
| whetherToLoadData         | whether to load input or reuse input: Y for yes, N for no            |
| whetherToCreateSet        | whether to create a new set to store input data: Y for yes, N for no |
| numImages                 | number of input images to generate for inferences         |


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


