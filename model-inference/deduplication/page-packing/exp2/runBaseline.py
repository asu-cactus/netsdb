import numpy as np
import sys
from numpy.lib.arraysetops import unique
import uuid
import hashlib
import timeit
import itertools
import pickle

# import the PagePacking.py under algorithm folder
import os
from sys import path
sys.path.append('../algorithms/')
from PagePacking import *

# pack all the tensors in different models together
list_of_tensors = list()

input_1 = np.load('../data/exp2_50*10000_64/m1.npy', allow_pickle=True).item()
block_size_1 = input_1.get('block_size')
tensor_shapes_1 = input_1.get('blocked_tensor_dimension')
tensor_mapping_1 = input_1.get('tensor_mapping')
num_tensors_1 = len(tensor_shapes_1)
#print(num_tensors_1)

for i in range (num_tensors_1):
    tensor_shapes_1[i] = input_1.get('blocked_tensor_dimension')[i]
for t in range(num_tensors_1):
    first, snd = tensor_shapes_1[t]
    l = set()
    for i in range(first):
        for j in range(snd):
            l.add(tensor_mapping_1[t].get((i,j)))
    list_of_tensors.append(l)

input_2 = np.load('../data/exp2_50*10000_64/m2.npy', allow_pickle=True).item()
block_size_2 = input_2.get('block_size')
tensor_shapes_2 = input_2.get('blocked_tensor_dimension')
tensor_mapping_2 = input_2.get('tensor_mapping')
num_tensors_2 = len(tensor_shapes_2)
#print(num_tensors_2)

for i in range (num_tensors_2):
    tensor_shapes_2[i] = input_2.get('blocked_tensor_dimension')[i]
for t in range(num_tensors_2):
    first, snd = tensor_shapes_2[t]
    l = set()
    for i in range(first):
        for j in range(snd):
            l.add(tensor_mapping_2[t].get((i,j)))
    list_of_tensors.append(l)

input_3 = np.load('../data/exp2_50*10000_64/m3.npy', allow_pickle=True).item()
block_size_3 = input_3.get('block_size')
tensor_shapes_3 = input_3.get('blocked_tensor_dimension')
tensor_mapping_3 = input_3.get('tensor_mapping')
num_tensors_3 = len(tensor_shapes_3)
#print(num_tensors_3)

for i in range (num_tensors_3):
    tensor_shapes_3[i] = input_3.get('blocked_tensor_dimension')[i]
for t in range(num_tensors_3):
    first, snd = tensor_shapes_3[t]
    l = set()
    for i in range(first):
        for j in range(snd):
            l.add(tensor_mapping_3[t].get((i,j)))
    list_of_tensors.append(l)

input_4 = np.load('../data/exp2_50*10000_64/m4.npy', allow_pickle=True).item()
block_size_4 = input_4.get('block_size')
tensor_shapes_4 = input_4.get('blocked_tensor_dimension')
tensor_mapping_4 = input_4.get('tensor_mapping')
blocks_id_max = (max(input_4.get('tensor_mapping')[3].values()))
num_tensors_4 = len(tensor_shapes_4)
#print(num_tensors_4)

for i in range (num_tensors_4):
    tensor_shapes_4[i] = input_4.get('blocked_tensor_dimension')[i]
for t in range(num_tensors_4):
    first, snd = tensor_shapes_4[t]
    l = set()
    for i in range(first):
        for j in range(snd):
            l.add(tensor_mapping_4[t].get((i,j)))
    list_of_tensors.append(l)
        
num_tensors = num_tensors_1 + num_tensors_2 + num_tensors_3 + num_tensors_4
#print(num_tensors)

output = dict()
output['list_of_tensors'] = list_of_tensors
#print(len(list_of_tensors))
np.save('../data/tensor_list_exp2.npy', output)

# load the input file
input = np.load('../data/tensor_list_exp2.npy', allow_pickle=True).item()
tensor_list_model = [{}, {}, {}, {}]
list_of_tensors_model = [[], [], [], []]

for num in range(4):
    a0=set(input['list_of_tensors'][0+num*4])
    a1=set(input['list_of_tensors'][1+num*4])
    a2=set(input['list_of_tensors'][2+num*4])
    a3=set(input['list_of_tensors'][3+num*4])
    tensor_list_model[num]=a0.union(a1, a2, a3)

# transfer the set to list
for i in range(4):
    list_of_tensors_model[i] = list(tensor_list_model[i])

"""
for i in range(len(list_of_tensors)):
    print('tensor: ', i)
    print(len(list_of_tensors[i]))
"""

list_of_first_tensors = list()
list_of_tensor_id = list()
for i in range(4):
    list_of_first_tensors.append(list_of_tensors[i*4])
    list_of_tensor_id.append(i*4)

list_of_private_tensors = list()
list_of_private_tensors_id = list()

for i in range(4):
    for j in range(1, 4):
        list_of_private_tensors.append(list_of_tensors[i*4+j])
        list_of_private_tensors_id.append(i*4+j)

# each page can have 2 blocks
blocks_in_page = 2

#print(len(list_of_tensors))

P = set()
# run the Baseline algorithm
start = timeit.default_timer()
P = bin_pack_base(list_of_first_tensors, blocks_in_page)
#P = bin_pack_base(tensor_list_model, blocks_in_page)
P_unique = bin_pack_private_blocks(list_of_private_tensors,list_of_private_tensors_id, blocks_in_page)
stop = timeit.default_timer()
L = list(P)
L_unique = list(P_unique)

# print the results
print('page packing latency: ', stop - start , ' seconds')
print('required number of pages of shared blocks: ', L[0].numBins, ' pages')
print('required number of pages of non-shared blocks: ', L_unique[0].numBins, ' pages')
print('required number of pages of all blocks: ', L[0].numBins + L_unique[0].numBins, ' pages')