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

# Create a 2D array to save the packing scheme
class BinPackingScheme(object):
    def __init__(self, item_ids, l):
        # Each row is a tensor
        # Each col is the bin/page
        self.p_i_j = [[0 for _ in range(math.ceil(len(item_ids) / l))] for _ in range(len(item_ids))]
        self.item_ids = list(item_ids)
        self.l = l
        self.numBins = math.ceil(len(item_ids) / l)

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

# ordering sizes for Greedy-2 algorithm
def order_tensors_by_size(T):
    return sorted(T, key=lambda x: len(x), reverse=True)

# ordering frequency for Greedy-2 algorithm
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
1:INPUT1: 𝑇 (a set of tensors, each tensor is a set of item ids i.e. tensor blocks ids)
2:INPUT2: 𝑙 (the maximum number of items for each bin)
3:OUTPUT: {𝑃𝑖𝑗} (an approximate optimal bin-packing scheme)
"""
def bin_pack_greedy(T, l):
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

def w2v_greedy1(tensor_list, blocks_in_page):

    # set the required number of pages as 0
    numBins = 0
    p_i_j = BinPackingScheme(tensor_list, blocks_in_page)

    l=tensor_list[0].intersection(tensor_list[1],tensor_list[2],tensor_list[3],tensor_list[4],tensor_list[5])
    p_i_j = BinPackingScheme(l, blocks_in_page)
    for i in range(len(l)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l) / blocks_in_page)

    l0=tensor_list[0]-l
    p_i_j = BinPackingScheme(l0, blocks_in_page)
    for i in range(len(l0)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l0) / blocks_in_page)

    l1=tensor_list[1]-l
    p_i_j = BinPackingScheme(l1, blocks_in_page)
    for i in range(len(l1)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l1) / blocks_in_page)

    l2=tensor_list[2]-l
    p_i_j = BinPackingScheme(l2, blocks_in_page)
    for i in range(len(l2)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l2) / blocks_in_page)

    l3=tensor_list[3]-l
    p_i_j = BinPackingScheme(l3, blocks_in_page)
    for i in range(len(l3)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l3) / blocks_in_page)

    l4=tensor_list[4]-l
    p_i_j = BinPackingScheme(l4, blocks_in_page)
    for i in range(len(l4)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l4) / blocks_in_page)

    l5=tensor_list[5]-l
    p_i_j = BinPackingScheme(l5, blocks_in_page)
    for i in range(len(l5)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l5) / blocks_in_page)

    return numBins

def w2v_twostage(tensor_list, blocks_in_page):

    # set the required number of pages as 0
    numBins = 0
    p_i_j = BinPackingScheme(tensor_list, blocks_in_page)

    l=tensor_list[0].intersection(tensor_list[1],tensor_list[2],tensor_list[3],tensor_list[4],tensor_list[5])
    p_i_j = BinPackingScheme(l, blocks_in_page)
    newBins = int(len(l) / blocks_in_page)
    numBins = numBins + newBins
    for i in range(blocks_in_page*newBins):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)

    l0=tensor_list[0]-l
    p_i_j = BinPackingScheme(l0, blocks_in_page)
    newBins0 = int(len(l0) / blocks_in_page)
    numBins = numBins + newBins0
    for i in range(blocks_in_page*newBins0):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)

    l1=tensor_list[1]-l
    p_i_j = BinPackingScheme(l1, blocks_in_page)
    newBins1 = int(len(l1) / blocks_in_page)
    numBins = numBins + newBins1
    for i in range(blocks_in_page*newBins1):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)

    l2=tensor_list[2]-l
    p_i_j = BinPackingScheme(l2, blocks_in_page)
    newBins2 = int(len(l2) / blocks_in_page)
    numBins = numBins + newBins2
    for i in range(blocks_in_page*newBins2):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)

    l3=tensor_list[3]-l
    p_i_j = BinPackingScheme(l3, blocks_in_page)
    newBins3 = int(len(l3) / blocks_in_page)
    numBins = numBins + newBins3
    for i in range(blocks_in_page*newBins3):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)

    l4=tensor_list[4]-l
    p_i_j = BinPackingScheme(l4, blocks_in_page)
    newBins4 = int(len(l4) / blocks_in_page)
    numBins = numBins + newBins4
    for i in range(blocks_in_page*newBins4):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)

    l5=tensor_list[5]-l
    p_i_j = BinPackingScheme(l5, blocks_in_page)
    newBins5 = int(len(l5) / blocks_in_page)
    numBins = numBins + newBins5
    for i in range(blocks_in_page*newBins5):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)

    # deal with the blocks which can not be packed into a full page
    new_tensor_list = [[], [], [], [], [], []]
    for i in range(len(l)-blocks_in_page*newBins):
        for j in range(6):
            new_tensor_list[j].append(i)
    block_id = len(l)-blocks_in_page*newBins
    block_increase = len(l0)-blocks_in_page*newBins0
    for k in range(6):
        for i in range(block_id+k*block_increase, block_id+(k+1)*block_increase):
            new_tensor_list[k].append(i)

    # apply the remaining blocks into Greedy-2 algorithm
    P = set()
    P, tensor_page_mapping = bin_pack_greedy(new_tensor_list, blocks_in_page)
    L = list(P)
    numBins = numBins + L[0].numBins
    return numBins

def bin_pack_base(T, l):
    I = set()
    for t_i in T:
        I = I.union(t_i)
    I = list(I)
    
    items = T[0]

    i, j = 0, 0
    p_i_j = BinPackingScheme(I, l)

    # Process at all items in t0
    for i in range(1, len(items) + 1):
        # Use 1-index according to logic
        j = I.index(items[i - 1]) + 1
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