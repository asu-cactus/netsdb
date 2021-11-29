import numpy as np
import sys
from numpy.lib.arraysetops import unique
import uuid
import hashlib
import timeit
import itertools

# import the PagePacking.py under algorithm folder
import os
from sys import path
sys.path.append('../algorithm/')
from PagePacking import *

# load the input file
input = np.load('../data/detector_output_same_size_unshared_located_random.npy', allow_pickle=True).item()
# get the information of input tensor lists
tensor_mapping = input.get('tensor_mapping')
tensor_shapes = input.get('blocked_tensor_dimension')
num_tensors = len(tensor_shapes)

# each page can have 8 blocks
blocks_in_page = 8

list_of_tensors = list()
for i in range (num_tensors):
    tensor_shapes[i] = input.get('blocked_tensor_dimension')[i]
for t in range(num_tensors):
    first, snd = tensor_shapes[t]
    l = list()
    for i in range(first):
        for j in range(snd):
            l.append(tensor_mapping[t].get((i,j)))
    list_of_tensors.append(l)

P = set()

# run the Baseline algorithm
start = timeit.default_timer()
P = bin_pack_base(list_of_tensors, blocks_in_page)
stop = timeit.default_timer()
L = list(P)

# print the results
print('page packing latency: ', stop - start , ' seconds')
print('required number of pages: ', L[0].numBins , ' pages')