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
input = np.load('../data/tensor_list.npy', allow_pickle=True).item()
tensor_list = [{}, {}, {}, {}, {}]
list_of_tensors = [[], [], [], [], []]
for num in range(5):
    a0=set(input['list_of_tensors'][0+num*5])
    a1=set(input['list_of_tensors'][1+num*5])
    a2=set(input['list_of_tensors'][2+num*5])
    a3=set(input['list_of_tensors'][3+num*5])
    a4=set(input['list_of_tensors'][4+num*5])
    tensor_list[num]=a0.union(a1, a2, a3, a4)

# transfer the set to list
for i in range(5):
    list_of_tensors[i] = list(tensor_list[i])
# each page can have 8 blocks
blocks_in_page = 8

P = set()
# run the Greedy-2 algorithm
start = timeit.default_timer()
P, tensor_page_mapping = bin_pack_greedy(list_of_tensors, blocks_in_page)
stop = timeit.default_timer()
L = list(P)

# print the results
print('page packing latency: ', stop - start , ' seconds')
print('required number of pages: ', L[0].numBins , ' pages')