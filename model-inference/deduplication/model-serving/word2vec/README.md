# Word2Vec

Deduplicated Word2Vec implementation based on matrix multiplication

## Example Application Source:

[Word2Vec Application Code](https://github.com/asu-cactus/netsdb/blob/master/src/tests/source/TestWord2VecWithDeduplication.cc)

## Compilation:

```
scons libword2vec
```

## Run:

Once you start the netsDB system [Start Cluster](https://github.com/asu-cactus/netsdb/blob/master/README.md), run following command:

```
bin/testWord2VecWithDeduplication loadData numModels
```


| Parameter         | Explanation                                               |
| ------------------|-----------------------------------------------------------|
| loadData          | Y for yes and N for no. if Y, it will create weights for each model and load the input feature vectors, otherwise, it will assume the models are created and the input features have been stored in the database         |
| numModels         | Number of Word2Vec models to be loaded or created         |

You can also modify the variables in [code](https://github.com/asu-cactus/netsdb/blob/master/src/tests/source/TestWord2VecWithDeduplication.cc) to control batch size and other word2vec parameters

| Variable                | Explanation                                               |
| ------------------------|-----------------------------------------------------------|
| batch\_size             | The number of input feature vectors for an inference batch|
| block\_x                | The size of the x dimension of a tensor block             |
| block\_y                | The size of the y dimension of a tensor block             |
| numBlock\_x             | The total number of blocks along the x dimension of the word2vec embedding tensor, block\_x * numBlock\_x = embedding\_vector\_dimension |
| matrix\_totalNumBlock\_y| The total number of blocks along the y dimension of the word2vec embedding tensor, block\_y * matrix\_totalNumBlock\_y = vocab\_size |
