# Text Classification

A text classification implementation with deduplication. 
The first layer is a word2vec embedding layer. 
The second layer is a fully-connected layer encapsulated in a UDF. 

## Example Application Source:

[Text Classification Application Code](https://github.com/asu-cactus/netsdb/blob/master/src/tests/source/TestSemanticClassificationWithDeduplication.cc)

## Compilation:

```
scons libclassifier
```

## Run:

Once you start the netsDB system [Start Cluster](https://github.com/asu-cactus/netsdb/blob/master/README.md), run following command:

```
bin/dedupClassifier whetherToLoadData numModels
```


| Parameter         | Explanation                                               |
| ------------------|-----------------------------------------------------------|
| loadData          | Y for yes and N for no. if Y, it will create weights for each model and load the input feature vectors, otherwise, it will assume the models are created and the input features have been stored in the database         |
| numModels         | Number of Word2Vec models to be loaded or created         |


You can modify the below variables in the example application source to control the block shape and the size of the word2vec layer, as well as the inference batch size.


| Variable                | Explanation                                               |
| ------------------------|-----------------------------------------------------------|
| batch\_size             | The number of input feature vectors for an inference batch|
| block\_x                | The size of the x dimension of a tensor block             |
| block\_y                | The size of the y dimension of a tensor block             |
| numBlock\_x             | The total number of blocks along the x dimension of the word2vec embedding tensor, block\_x * numBlock\_x = embedding\_vector\_dimension |
| matrix\_totalNumBlock\_y| The total number of blocks along the y dimension of the word2vec embedding tensor, block\_y * matrix\_totalNumBlock\_y = vocab\_size |


To customize the fully-connected layer, you need modify the encapsulated UDF code: [UDF code](https://github.com/asu-cactus/netsdb/blob/master/src/word2vec/headers/SemanticClassifier.h)
