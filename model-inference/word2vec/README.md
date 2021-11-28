# Word2Vec

Word2Vec implementation based on matrix multiplication

## Example Application Source:

[Word2Vec Application Code](https://github.com/asu-cactus/netsdb/blob/master/src/word2vec/source/Word2Vec.cc)

## Compilation:

```
scons libword2vec
```

## Run:

### Command:

```
bin/word2vec blockDimensionX blockDimensionY whetherToLoadData numModels path/to/embedding_weights(leave empty if generate random)
```

### Parameters:

| Parameter         | Explanation                                               |
| ------------------|-----------------------------------------------------------|
| blockDimensionX   | The size of the x dimension of a 2D tensor block          |
| blockDimensionY   | The size of the y dimension of a 2D tensor block          |
| whetherToLoadData | Y for yes, N for no                                       |
| numModels         | Number of Word2Vec models to be loaded or created         |
| path/to/embedding\_weights| path to model files that needs to be loaded, <br /> if left empty, the models will be created randomly                                 |
