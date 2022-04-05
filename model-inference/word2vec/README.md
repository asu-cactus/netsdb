# Word2Vec

Word2Vec implementation based on matrix multiplication

## Example Application Source:

[Word2Vec Application Code](https://github.com/asu-cactus/netsdb/blob/master/src/word2vec/source/Word2Vec.cc)

## Compilation:

```
scons libword2vec
```

## Run:

Once you start the netsDB system [Start Cluster](https://github.com/asu-cactus/netsdb/blob/master/README.md), run following command:

```
bin/word2vec blockDimensionX blockDimensionY whetherToLoadData numModels path/to/embedding_weights(leave empty if generate random)
```


| Parameter         | Explanation                                               |
| ------------------|-----------------------------------------------------------|
| blockDimensionX   | The size of the x dimension of a 2D tensor block          |
| blockDimensionY   | The size of the y dimension of a 2D tensor block          |
| whetherToLoadData | Y for yes, N for no                                       |
| numModels         | Number of Word2Vec models to be loaded or created         |
| path/to/embedding\_weights| path to model files that needs to be loaded, <br /> if left empty, the models will be created randomly|

You can also modify the variables in [code](https://github.com/asu-cactus/netsdb/blob/master/src/word2vec/source/Word2Vec.cc) to control batch size and other word2vec parameters

| Variable             | Explanation                                               |
| ---------------------|-----------------------------------------------------------|
| batch\_size          | The number of input feature vectors for an inference batch|
| vocab\_size          | The size of the vocabulary                                |
| embedding\_dimension | The size of an embedding vector                           |
