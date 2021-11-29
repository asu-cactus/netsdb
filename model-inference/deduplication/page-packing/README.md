# Packing Tensor Blocks to Database Pages
This repository contains all the code for page packing testing including 4 algorithms (Baseline, Two-Stage, Greedy-1 and Greedy-2) on the following 4 testing cases:
1 word2vec (100 * 10000, 64MB)
2 text classification (100 * 10000, 64MB)
3 text classification (300 * 300, 64MB)
4 text classification (300 * 300, 32MB)

## Algorithms
All the 4 algorithms (Baseline, Two-Stage, Greedy-1 and Greedy-2) are in PagePacking.py under the algorithm folder.

## word2vec (100 * 10000, 64MB)
For word2vec (100 * 10000, 64MB) testing, please go to word2vec folder and there are 4 python scripts which are Baseline.py, Two-Stage.py, Greedy-1.py and Greedy-2.py.

The input of these 4 Python scripts is a dictionary object saved in data/detector_output_same_size_unshared_located_random.npy file which has the following format:

Key: Value

list_blocks: a list that stores the distinct blocks, the size is equals to num_distinct_blocks.

tensor_mapping: a list of tensor mapping whose length is the number of tensor. Each element is a dictionary that stores the mapping between the block index and its distinct block ID. e.g tensor_mapping[0][(1,1)] = 50 means the block (1,1) in the 0th tensor is mapped to the distinct block 50.
blocked_tensor_dimension: a list of dimensions for each blocked tensor
block_size: the size of the block

detector_output_same_size_unshared_located_random.npy has 6 tensors and each tensor is in the same dimension after blocking, (5,100). There are 50 non-shareable blocks for each tensor and these blocks are randomly distributed. The lower-bound for this case is ⌈750/8⌉ = 94 pages.

You can directly run the following 4 python scripts to get the page packing latency and required number of pages using different page packing algorithms. 4 functions corresponding to 4 algorithms can be found in PagePacking.py under the algorithm folder. The function names are bin_pack_base for Baseline, w2v_greedy1 for Greedy-1, bin_pack_greedy for Greedy-2 and w2v_twostage for Two-Stage.
```
$python3 Baseline.py
$python3 Two-Stage.py
$python3 Greedy-1.py
$python3 Greedy-2.py
```

## text classification (100 * 10000, 64MB)
For text classification (100 * 10000, 64MB) testing, please go to text_classification_100 folder and there are 4 python scripts which are Baseline.py, Two-Stage.py, Greedy-1.py and Greedy-2.py.

The input of these 4 Python scripts is a dictionary object saved in data/tensor_list.npy file. It only has one key, which is 'list_of_tensors' and you can get the list of all tensors from 5 models via the following python scripts.
```
import numpy as np
output = np.load('tensor_list.npy', allow_pickle=True).item()
list_of_tensors = output['list_of_tensors']
```

All the tensors are from 5 models. The original tensors for these 5 models also can be found under the data folder as .npy files. The following is a list tensors of these 5 models.
1 civil_trainable.npy
2 imdb_nontrainable.npy
3 imdb_trainable.npy
4 yelp_nontrainable.npy
5 yelp_trainable.npy

You can directly run the following 4 python scripts to get the page packing latency and required number of pages using different page packing algorithms. 4 functions corresponding to 4 algorithms can be found in PagePacking.py under the algorithm folder.
```
$python3 Baseline.py
$python3 Two-Stage.py
$python3 Greedy-1.py
$python3 Greedy-2.py
```

## text classification (300 * 300, 64MB)
For text classification (300 * 300, 64MB) testing, please go to text_classification_300_64 folder and there are 4 python scripts which are Baseline.py, Two-Stage.py, Greedy-1.py and Greedy-2.py. The inputs are the same as text classification (100 * 10000, 64MB). You can directly run the following 4 python scripts to get the page packing latency and required number of pages using different page packing algorithms. 4 functions corresponding to 4 algorithms can be found in PagePacking.py under the algorithm folder.
```
$python3 Baseline.py
$python3 Two-Stage.py
$python3 Greedy-1.py
$python3 Greedy-2.py
```

## text classification (300 * 300, 32MB)
For text classification (300 * 300, 32MB) testing, please go to text_classification_300_32 folder and there are 4 python scripts which are Baseline.py, Two-Stage.py, Greedy-1.py and Greedy-2.py. The inputs are the same as text classification (100 * 10000, 64MB). You can directly run the following 4 python scripts to get the page packing latency and required number of pages using different page packing algorithms. 4 functions corresponding to 4 algorithms can be found in PagePacking.py under the algorithm folder.
```
$python3 Baseline.py
$python3 Two-Stage.py
$python3 Greedy-1.py
$python3 Greedy-2.py
```