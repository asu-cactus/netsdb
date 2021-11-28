# Text Classification

A text classification implementation. 
The first layer is a word2vec embedding layer. 
The second layer is a fully-connected layer encapsulated in a UDF. 

## Example Application Source:

[Text Classification Application Code](https://github.com/asu-cactus/netsdb/blob/master/src/tests/source/TestSemanticClassifier.cc)

## Compilation:

```
scons libclassifier
```

## Run:

Once you start the netsDB system [Start Cluster](https://github.com/asu-cactus/netsdb/blob/master/README.md), run following command:

```
bin/classifier whetherToLoadData numModels
```

You can modify the below variables in the example application source to control the block shape and the size of the word2vec layer, as well as the inference batch size.

| Parameter/Variables | Explanation                                               |
| ------------------- |-----------------------------------------------------------|
| block\_x            | The size of the x dimension of a 2D tensor block          |
| block\_y            | The size of the y dimension of a 2D tensor block          |
| numBlock\_x         | The number of blocks along the x dimension                |
| numBlock\_y         | The number of blocks along the y dimension                |
| batch\_size         | The number of input feature vectors for an inference batch|
| whetherToLoadData   | Y for yes, N for no                                       |
| numModels           | Number of models to be created for experiments            |

To customize the fully-connected layer, you need modify the encapsulated UDF code: [UDF code](https://github.com/asu-cactus/netsdb/blob/master/src/word2vec/headers/SemanticClassifier.h)
