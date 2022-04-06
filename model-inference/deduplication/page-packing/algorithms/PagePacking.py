from array import array
import math

from numpy.lib.arraysetops import isin
from sympy.utilities.iterables import multiset_permutations
from sympy.utilities.iterables import multiset_combinations
import itertools
from math import factorial
import hashlib
import numpy as np
import gc
import csv
import pickle
from itertools import combinations

"""
A class to store the packing scheme.
Each row represents a tensor and each column represnts a page
0 means that the blcok is not in the corresponding page. 1 just the opposite.
Also includes the number of pages needed
"""
class BinPackingScheme(object):
    # initialization
    def __init__(self, item_ids, l):
        # Each row is a tensor
        # Each col is the bin/page
        self.p_i_j = [[0 for _ in range(math.ceil(len(item_ids) / l))] for _ in range(len(item_ids))]
        self.item_ids = list(item_ids)
        self.l = l
        self.numBins = math.ceil(len(item_ids) / l)

    # Determine whether it has been marked.
    # 0 means that the blcok is not in the corresponding page. 1 just the opposite.
    def is_marked(self, item_id):
        return any([x == 1 for x in self.p_i_j[self.item_ids.index(item_id)]])

    def __eq__(self, other):
        my_array_hash = "".join([str(j) for sub in self.p_i_j for j in sub])
        other_array_hash = "".join([str(j) for sub in other.p_i_j for j in sub])
        if my_array_hash != other_array_hash:
            return False
        
        if len(self.item_ids) != len(other.item_ids):
            return False

        if self.numBins != other.numBins:
            return False

        # Order of items is also important
        for i in range(len(self.item_ids)):
            if self.item_ids[i] != other.item_ids[i]:
                return False

        return True

    def __ne__(self, other):
        my_array_hash = "".join([str(j) for sub in self.p_i_j for j in sub])
        other_array_hash = "".join([str(j) for sub in other.p_i_j for j in sub])
        if my_array_hash != other_array_hash:
            return True
        
        if len(self.item_ids) != len(other.item_ids):
            return True

        if self.numBins != other.numBins:
            return True

        # Order of items is also important
        for i in range(len(self.item_ids)):
            if self.item_ids[i] != other.item_ids[i]:
                return True

        return False

    def __hash__(self):
        """
        This is important. If this does not work, we cannot tell the difference between two bin packing schemes.
        What identifies a bin packing scheme is:
        1. The items being packed, i.e. the items must be uniquely identifiable
        2. The sequAmount of space savedence of packing pages into bins 
        """
        array_hash = "".join([str(j) for sub in self.p_i_j for j in sub])
        items_hash = "".join([str(hash(i)) for i in self.item_ids])
        full_hash = (array_hash + items_hash).encode("utf-8")

        return int(hashlib.md5(full_hash).hexdigest(), 16)

    # The marked function replaces the original 0 with 1
    def mark(self, i, j):
        if j - 1 > len(self.p_i_j[0]) - 1:
            diff = (j - 1) - (len(self.p_i_j[0]) - 1)
            # Add new bucket
            for row in self.p_i_j:
                row.extend([0 for _ in range(diff)])

        # Convert 1-index to 0-index
        self.p_i_j[i - 1][j - 1] = 1

    def merge(self, otherBinPackingScheme):
        assert self.l == otherBinPackingScheme.l

        for i in range(len(self.item_ids)):
            self.p_i_j[i] = self.p_i_j[i] + [0 for _ in range(otherBinPackingScheme.numBins)]

        # Take care of common item ids
        common_items = set(self.item_ids).intersection(set(otherBinPackingScheme.item_ids))
        for item in common_items:
            our_index = self.item_ids.index(item)
            their_index = otherBinPackingScheme.item_ids.index(item)
            self.p_i_j[our_index] += otherBinPackingScheme.p_i_j[their_index]

        # Take care of new item ids
        our_index = len(self.item_ids) - 1
        new_items = []
        for other_index, item in enumerate(otherBinPackingScheme.item_ids):
            if item in common_items:
                continue

            our_index += 1
            new_items.append(item)
            new_row = [0 for _ in range(self.numBins)] + otherBinPackingScheme.p_i_j[other_index]
            self.p_i_j.append(new_row)

        self.numBins += otherBinPackingScheme.numBins
        self.item_ids.extend(new_items)

        return self

    def blocks_in_bin_id(self, bin_id):
        return [self.item_ids[k] for k in range(len(self.item_ids)) if self.p_i_j[k][bin_id] == 1]

    """
    INPUT1: all_bins (a set of bins, each bin representing a page of tensor blocks)
    INPUT2: t (a set of tensor blocks)
    OUTPUT: bin_set (a minimum set of bins that maximally cover t)
    """
    def findMinBinsMaxCover(self, t, l):
        # A set of item ids
        T = set(t)
        # will contain groups of item ids
        bin_set = set()
        used_bins = set()

        while T:
            cover = l
            bin = None
            bin_id = None
            for j in range(self.numBins):
                if j in used_bins:
                    continue

                # Intersect tensor items in T with the items present in bin j
                bin_items = frozenset(self.blocks_in_bin_id(j))
                new_cover = len(T.intersection(bin_items))
                if new_cover == cover:
                    cover = new_cover
                    bin = bin_items
                    bin_id = j

            # If we have bins but their contents dont cover t at all i.e. different items
            if not bin:
                break

            used_bins.add(bin_id)
            bin_set = bin_set.union(T.intersection(bin))
            T = T - bin
            # All bins used
            if len(used_bins) == self.numBins:
                break

        return bin_set, used_bins

# Ordering sizes for Greedy-2 algorithm
# Sort the tensors from large to small according to the size
def order_tensors_by_size(T):
    return sorted(T, key=lambda x: len(x), reverse=True)

# Ordering frequency for Greedy-2 algorithm
# Sort from high to low according to the frequency of occurrence in all tensors
def order_tensor_blocks_by_freq(T, t_i):
    freq_map = {}
    for block in t_i:
        if not block in freq_map:
            freq_map[block] = 1
        else:
            freq_map[block] += 1

    for t_k in T:
        if not isinstance(t_i, list) and t_k == t_i:
            continue

        for block in t_k:
            if block in freq_map:
                freq_map[block] += 1

    ordered_items = sorted(freq_map.items(), key=lambda x: x[1], reverse=True)
    return [x[0] for x in ordered_items]

"""
1:INPUT1: tensor_list (a set of tensors, each tensor is a set of item ids i.e. tensor blocks ids)
2:INPUT2: blocks_in_page (the maximum number of blocks for each page)
3:OUTPUT: {𝑃𝑖𝑗} (an approximate optimal bin-packing scheme) including number of pages needed
Used in the Greedy-1 algorithm in all 9 test cases:
1. word2vec (100 * 10000, 64MB)
2. text classification (100 * 10000, 64MB)
3. text classification (300 * 300, 64MB)
4. text classification (300 * 300, 32MB)
5. exp1 (50 * 10000, 64MB)
6. exp2 (50 * 10000, 64MB)
7. exp3-pair1 (50 * 10000, 64MB)
8. exp3-pair2 (50 * 10000, 64MB)
9. exp3-pair3 (50 * 10000, 64MB)
These testing examples can be found in runGreedy-2.py in the corresponding 9 folders
"""
def bin_pack_greedy1(tensor_list, tensor_id_list, blocks_in_page, blocks_id_max):
    
    I = set()
    for t_i in tensor_list:
        I = I.union(t_i)
    I = list(I)

    total_block_set = set()
    for i in range(blocks_id_max+1):
        total_block_set.add(i)

    tuple_list = list()
    tuple_name_list = list()
    block_list = list()
    num_tensors = len(tensor_list)
    tensor_position_array = np.arange(num_tensors)

    for i in range(num_tensors+1,0,-1):
        temp = combinations(tensor_position_array.tolist(),i)
        temp_list = list(temp)
        temp_name = combinations(tensor_id_list,i)
        temp_list_name = list(temp_name)
        for j in range(len(temp_list)):
            tuple_list.append(temp_list[j])
            tuple_name_list.append(temp_list_name[j])
    
    p_i_j = BinPackingScheme(I, blocks_in_page)

    output_model_block_map = dict()

    # set the required number of pages as 0
    numBins = 0

    # create a list of equivalent class tensors
    equivalent_class_tensors = []

    # divide blocks into multiple equivalent classes
    for i in range(len(tuple_list)):
        this_tuple_name = tuple_name_list[i]
        this_tuple = tuple_list[i]
        this_tuple_size = len(this_tuple)
        if(this_tuple_size == 1):
            this_set = tensor_list[this_tuple[0]]
        elif(this_tuple_size == 2):
            this_set = tensor_list[this_tuple[0]].intersection(tensor_list[this_tuple[1]])
        else:
            this_set = tensor_list[this_tuple[0]].intersection(tensor_list[this_tuple[1]])
            for j in range(2,this_tuple_size):
                this_set = this_set.intersection(tensor_list[this_tuple[j]])
        this_equivalent_class_block = this_set.intersection(total_block_set)
        total_block_set = total_block_set - this_equivalent_class_block
        this_list = list(this_equivalent_class_block)
        block_list.append(this_list)
        equivalent_class_tensors.append(this_list)
        if(len(this_list) != 0):
            output_model_block_map[''.join(str(this_tuple_name))] = this_list
    
    #print(len(output_model_block_map))

    np.save('file2.npy', output_model_block_map)

    fileName="file2.csv"
    with open(fileName,"w") as csv_file:
        writer=csv.writer(csv_file)
        for key,value in output_model_block_map.items():
            writer.writerow([key,value])

    #print('finish writing to the files')

    # pack the blocks based on the equivalent classes
    for k in range(len(equivalent_class_tensors)):
        this_len = len(equivalent_class_tensors[k])
        if(this_len == 0):
            continue
        this_ten = equivalent_class_tensors[k]
        for i in range(this_len):
            j = I.index(this_ten[i]) + 1
            s = numBins + math.ceil(i / blocks_in_page)
            p_i_j.mark(j, s)
        #print(math.ceil(this_len / blocks_in_page))
        numBins = numBins + math.ceil(this_len / blocks_in_page)
        p_i_j.numBins = numBins

    # return the bin-packing scheme and dict of mapping
    return set([p_i_j]), output_model_block_map

"""
1:INPUT1: T (a set of tensors, each tensor is a set of item ids i.e. tensor blocks ids)
2:INPUT2: l (the maximum number of blocks for each page)
3:OUTPUT: {𝑃𝑖𝑗} (an approximate optimal bin-packing scheme) including number of pages needed and mapping of tensor IDs and page IDs
Used in the Greedy-2 algorithm in all 9 test cases:
1. word2vec (100 * 10000, 64MB)
2. text classification (100 * 10000, 64MB)
3. text classification (300 * 300, 64MB)
4. text classification (300 * 300, 32MB)
5. exp1 (50 * 10000, 64MB)
6. exp2 (50 * 10000, 64MB)
7. exp3-pair1 (50 * 10000, 64MB)
8. exp3-pair2 (50 * 10000, 64MB)
9. exp3-pair3 (50 * 10000, 64MB)
These testing examples can be found in runGreedy-2.py in the corresponding 9 folders
"""
def bin_pack_greedy2(T, l):
    I = set()
    for t_i in T:
        I = I.union(t_i)
    I = list(I)

    tensor_page_mapping = dict()
    tensor_page_set = set()
    
    tensors = order_tensors_by_size(T)
    items = order_tensor_blocks_by_freq(T, tensors[0])
    
    i, j = 0, 0
    p_i_j = BinPackingScheme(I, l)

    # Process at all items in t0
    for i in range(1, len(items) + 1):
        # Use 1-index according to logic
        j = I.index(items[i - 1]) + 1
        s = math.ceil(i / l)
        tensor_page_set.add(s-1)
        p_i_j.mark(j, s)

    numBins = math.ceil(len(items) / l)
    p_i_j.numBins = numBins
    tensor_page_mapping[0] = tensor_page_set

    # Already added tensor t1
    for i in range(2, len(T) + 1):
        bin_set, used_bins = p_i_j.findMinBinsMaxCover(tensors[i - 1],l)
        tensor_page_set = used_bins
        I_delta = set(tensors[i - 1]) - bin_set
        I_delta = list(I_delta)

        if not I_delta:
            continue
        else:
            remaining_items = order_tensor_blocks_by_freq(T, I_delta)
            for j in range(1, len(remaining_items) + 1):
                # Important to index using I because we built BinPackingScheme using ordering of blocks in I
                s = I.index(remaining_items[j - 1]) + 1
                u = numBins + math.ceil(j / l)
                tensor_page_set.add(u-1)
                p_i_j.mark(s, u)

            numBins = numBins + math.ceil(len(remaining_items) / l)
            p_i_j.numBins = numBins
        tensor_page_mapping[i-1] = tensor_page_set

    return set([p_i_j]), tensor_page_mapping
    
"""
1:INPUT1: tensor_list (a set of tensors, each tensor is a set of item ids i.e. tensor blocks ids)
2:INPUT2: blocks_in_page (the maximum number of blocks for each page)
3:OUTPUT: {𝑃𝑖𝑗} (an approximate optimal bin-packing scheme) including number of pages needed
Used in the Two-Stage algorithm in the following 9 test cases:
1. word2vec (100 * 10000, 64MB)
2. text classification (100 * 10000, 64MB)
3. text classification (300 * 300, 64MB)
4. text classification (300 * 300, 32MB)
5. exp1 (50 * 10000, 64MB)
6. exp2 (50 * 10000, 64MB)
7. exp3-pair1 (50 * 10000, 64MB)
8. exp3-pair2 (50 * 10000, 64MB)
9. exp3-pair3 (50 * 10000, 64MB)
These testing examples can be found in runTwo-Stage.py in the corresponding 9 folders
"""
def bin_pack_twostage(tensor_list, tensor_id_list, blocks_in_page, blocks_id_max):

    I = set()
    for t_i in tensor_list:
        I = I.union(t_i)
    I = list(I)

    total_block_set = set()
    for i in range(blocks_id_max+1):
        total_block_set.add(i)

    p_i_j = BinPackingScheme(I, blocks_in_page)

    # set the required number of pages as 0
    numBins = 0

    # create a new tensor list for stage-2
    new_tensor_list = list()
    for k in range(len(tensor_list)):
        empty_set = list()
        new_tensor_list.append(empty_set)

    block_id = 0

    tuple_list = list()
    tuple_name_list = list()
    block_list = list()
    num_tensors = len(tensor_list)
    tensor_position_array = np.arange(num_tensors)

    for i in range(num_tensors+1,0,-1):
        temp = combinations(tensor_position_array.tolist(),i)
        temp_list = list(temp)
        temp_name = combinations(tensor_id_list,i)
        temp_list_name = list(temp_name)
        for j in range(len(temp_list)):
            tuple_list.append(temp_list[j])
            tuple_name_list.append(temp_list_name[j])
            
    output_model_block_map = dict()

    # create a list of equivalent class tensors
    equivalent_class_tensors = []

    # divide blocks into multiple equivalent classes
    for i in range(len(tuple_list)):
        this_tuple_name = tuple_name_list[i]
        this_tuple = tuple_list[i]
        this_tuple_size = len(this_tuple)
        if(this_tuple_size == 1):
            this_set = tensor_list[this_tuple[0]]
        elif(this_tuple_size == 2):
            this_set = tensor_list[this_tuple[0]].intersection(tensor_list[this_tuple[1]])
        else:
            this_set = tensor_list[this_tuple[0]].intersection(tensor_list[this_tuple[1]])
            for j in range(2,this_tuple_size):
                this_set = this_set.intersection(tensor_list[this_tuple[j]])
        this_equivalent_class_block = this_set.intersection(total_block_set)
        total_block_set = total_block_set - this_equivalent_class_block
        this_list = list(this_equivalent_class_block)
        block_list.append(this_list)
        equivalent_class_tensors.append(this_list)
        this_numBins = int(len(this_equivalent_class_block) / blocks_in_page)
        for i in range(len(this_equivalent_class_block)):
            j = I.index(this_list[i]) + 1
            s = numBins + math.ceil(i / blocks_in_page)
            p_i_j.mark(j, s)
        numBins = numBins + this_numBins
        p_i_j.numBins = numBins
        this_remain = len(this_equivalent_class_block) - blocks_in_page*this_numBins
        if (this_remain!=0):
            for i in range(block_id, block_id + this_remain):
                for j in range(len(this_tuple)):
                    new_tensor_list[this_tuple[j]].append(i)
        block_id = block_id + this_remain
        if(len(this_list) != 0):
            output_model_block_map[''.join(str(this_tuple_name))] = this_list

    # start the second stage
    P = set()
    P, tensor_page_mapping = bin_pack_greedy2(new_tensor_list, blocks_in_page)
    L = list(P)
    L[0].numBins = numBins + L[0].numBins
    P = set(L)
    P.union(set([p_i_j]))
    return P

"""
1:INPUT1: T (a set of tensors, each tensor is a set of item ids i.e. tensor blocks ids)
2:INPUT2: l (the maximum number of blocks for each page)
3:OUTPUT: {𝑃𝑖𝑗} (an approximate optimal bin-packing scheme) including number of pages needed
Used in the Baseline algorithm in all 9 test cases:
1. word2vec (100 * 10000, 64MB)
2. text classification (100 * 10000, 64MB)
3. text classification (300 * 300, 64MB)
4. text classification (300 * 300, 32MB)
5. exp1 (50 * 10000, 64MB)
6. exp2 (50 * 10000, 64MB)
7. exp3-pair1 (50 * 10000, 64MB)
8. exp3-pair2 (50 * 10000, 64MB)
9. exp3-pair3 (50 * 10000, 64MB)
These testing examples can be found in runBaseline.py in the corresponding 9 folders
"""
def bin_pack_base(T, l):

    I = set()
    for t_i in T:
        I = I.union(t_i)
    I = list(I)
    
    items = T[0]
    items_list = list(items)

    i, j = 0, 0
    p_i_j = BinPackingScheme(I, l)

    # Process at all items in t0
    for i in range(1, len(items) + 1):
        # Use 1-index according to logic
        j = I.index(items_list[i - 1]) + 1
        s = math.ceil(i / l)
        p_i_j.mark(j, s)

    numBins = math.ceil(len(items) / l)

    # Already added tensor t1
    for i in range(2, len(T) + 1):
        bin_set, used_bin = p_i_j.findMinBinsMaxCover(T[i - 1],l)
        I_delta = set(T[i - 1]) - bin_set
        I_delta = list(I_delta)

        if not I_delta:
            continue
        else:
            remaining_items = order_tensor_blocks_by_freq(T, I_delta)
            for j in range(1, len(remaining_items) + 1):
                # Important to index using I because we built BinPackingScheme using ordering of blocks in I
                s = I.index(remaining_items[j - 1]) + 1
                u = numBins + math.ceil(j / l)
                p_i_j.mark(s, u)

            numBins = numBins + math.ceil(len(remaining_items) / l)
            p_i_j.numBins = numBins
    return set([p_i_j])

def bin_pack_private_blocks(T, T_id, l):

    I = set()
    for t_i in T:
        I = I.union(t_i)
    I = list(I)
    
    items = T[0]

    i, j = 0, 0
    p_i_j = BinPackingScheme(I, l)

    output_model_block_map = dict()
    tensor_id_tuple = list()
    
    temp = combinations(T_id,1)
    temp_list = list(temp)
    for j in range(len(temp_list)):
        tensor_id_tuple.append(temp_list[j])
        
    for i in range(len(T)):
        this_tensor = list(T[i])
        this_tensor_id = tensor_id_tuple[i]
        output_model_block_map[''.join(str(this_tensor_id))] = this_tensor

    np.save('file2_private.npy', output_model_block_map)
    
    fileName="file2_private.csv"
    with open(fileName,"w") as csv_file:
        writer=csv.writer(csv_file)
        for key,value in output_model_block_map.items():
            writer.writerow([key,value])

    numBins = 0
    for i in range(len(T)):
        this_unique_tensor = T[i]
        this_unique_list = list(this_unique_tensor)
        for j in range(1, len(this_unique_list) + 1):
            s = I.index(this_unique_list[j - 1]) + 1
            u = numBins + math.ceil(j / l)
            p_i_j.mark(s, u)
        numBins = numBins + math.ceil(len(this_unique_list) / l)
        p_i_j.numBins = numBins
    return set([p_i_j])