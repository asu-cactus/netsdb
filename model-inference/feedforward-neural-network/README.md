# Feed-forward Neural Network

A feed-forward neural network implementation. It has two fully-connected hidden layers.


## Relation-Centric Implementation
### Example Application Source:

[Relation-Centric Feed-forward Neural Network Application Code](https://github.com/asu-cactus/netsdb/blob/master/src/tests/source/FFTest.cc)

### Compilation:

```
scons libFFTest
```

### Run:

Once you start the netsDB system [Start Cluster](https://github.com/asu-cactus/netsdb/blob/master/README.md), run following command:

```
bin/FFTest blockDimensionX blockDimensionY whetherToLoadData path/to/weights/and/bias(leave empty if generate random)
```

You can pass the command line parameters or modify variables in the example application source to control the inference batch size, number of labels, and neurons at each layer.

| Parameter/Variables       | Explanation                                               |
| ------------------------- |-----------------------------------------------------------|
| blockDimensionX           | The size of the x dimension of a 2D tensor block          |
| blockDimensionY           | The size of the y dimension of a 2D tensor block          |
| whetherToLoadData         | Y for yes, N for no                                       |
| path/to/embedding\_weights| path to model files that needs to be loaded, <br /> if left empty, the models will be created randomly|
| batch\_size               | number of input feature vectors in an inference batch     |
| features                  | number of features in each input feature vector           |
| hid1\_size                | number of neurons in the first hidden layer               |
| hid2\_size                | number of neurons in the second hidden layer              |
| num\_labels               | number of labels used for the classification task         |

Variables can be directly modified [here](https://github.com/asu-cactus/netsdb/blob/master/src/tests/source/FFTest.cc). Needs to recompile after modifying the variables.


## UDF-Centric Implementation
### Example Application Source:

[UDF-Centric Feed-forward Neural Network Application Code](https://github.com/asu-cactus/netsdb/blob/master/src/tests/source/FCProjTest.cc)

### Compilation:

```
scons libclassifier
```

### Run:

Once you start the netsDB system [Start Cluster](https://github.com/asu-cactus/netsdb/blob/master/README.md), run following command:

```
bin/FCProjTest blockDimensionX blockDimensionY whetherToLoadData path/to/weights/and/bias(leave empty if generate random)
```


