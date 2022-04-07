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
sys.path.append('../algorithms/')
from PagePacking import *

# pack all the tensors in different models together
list_of_tensors = list()

input_1 = np.load('../data/exp3-pair1_50*10000_64/m1.npy', allow_pickle=True).item()
block_size_1 = input_1.get('block_size')
#unique_blocks_1 = len(input_1.get('list_blocks'))
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

input_2 = np.load('../data/exp3-pair1_50*10000_64/m2.npy', allow_pickle=True).item()
block_size_2 = input_2.get('block_size')
tensor_shapes_2 = input_2.get('blocked_tensor_dimension')
tensor_mapping_2 = input_2.get('tensor_mapping')
blocks_id_max = (max(input_2.get('tensor_mapping')[3].values()))
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
        
num_tensors = num_tensors_1 + num_tensors_2
#print(num_tensors)

"""
output = dict()
output['list_of_tensors'] = list_of_tensors
#print(len(list_of_tensors))
np.save('../data/tensor_list_exp3-pair1.npy', output)

# load the input file
input = np.load('../data/tensor_list_exp3-pair1.npy', allow_pickle=True).item()
tensor_list_model = [{}, {}]
list_of_tensors = [[], []]

a0=set(input['list_of_tensors'][0])
a1=set(input['list_of_tensors'][1])
a2=set(input['list_of_tensors'][2])
a3=set(input['list_of_tensors'][3])
tensor_list_model[0]=a0.union(a1,a2,a3)

a4=set(input['list_of_tensors'][4])
a5=set(input['list_of_tensors'][5])
a6=set(input['list_of_tensors'][6])
a7=set(input['list_of_tensors'][7])
a8=set(input['list_of_tensors'][8])
tensor_list_model[1]=a4.union(a5,a6,a7,a8)

# transfer the set to list
for i in range(2):
    list_of_tensors[i] = list(tensor_list_model[i])
"""
"""
for i in range(len(list_of_tensors)):
    print('tensor: ', i)
    print(len(list_of_tensors[i]))
"""
"""
list_of_first_tensors = list()
list_of_tensor_id = list()
list_of_first_tensors.append(list_of_tensors[0])
list_of_tensor_id.append(0)
list_of_first_tensors.append(list_of_tensors[4])
list_of_tensor_id.append(4)

list_of_private_tensors = list()
list_of_private_tensors_id = list()

list_of_private_tensors.append(list_of_tensors[1])
list_of_private_tensors_id.append(1)
list_of_private_tensors.append(list_of_tensors[2])
list_of_private_tensors_id.append(2)
list_of_private_tensors.append(list_of_tensors[3])
list_of_private_tensors_id.append(3)
list_of_private_tensors.append(list_of_tensors[5])
list_of_private_tensors_id.append(5)
list_of_private_tensors.append(list_of_tensors[6])
list_of_private_tensors_id.append(6)
list_of_private_tensors.append(list_of_tensors[7])
list_of_private_tensors_id.append(7)
list_of_private_tensors.append(list_of_tensors[8])
list_of_private_tensors_id.append(8)
"""
# each page can have 2 blocks
blocks_in_page = 2

#print(len(list_of_tensors))

list_of_tensor_id = list(np.arange(9))

P = set()
# run the Greedy-1 algorithm
start = timeit.default_timer()
#P, output_model_block_map = bin_pack_greedy1(list_of_first_tensors, list_of_tensor_id, blocks_in_page, blocks_id_max)
#P_unique = bin_pack_private_blocks(list_of_private_tensors,list_of_private_tensors_id, blocks_in_page)
P, output_model_block_map = bin_pack_greedy1(list_of_tensors, list_of_tensor_id, blocks_in_page, blocks_id_max)
stop = timeit.default_timer()
L = list(P)
#L_unique = list(P_unique)

# print the results
print('page packing latency: ', stop - start , ' seconds')
print('required number of pages of shared blocks: ', L[0].numBins, ' pages')
#print('required number of pages of non-shared blocks: ', L_unique[0].numBins, ' pages')
#print('required number of pages of all blocks: ', L[0].numBins + L_unique[0].numBins, ' pages')