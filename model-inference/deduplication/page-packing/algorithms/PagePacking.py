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
1:INPUT1: T (a set of tensors, each tensor is a set of item ids i.e. tensor blocks ids)
2:INPUT2: l (the maximum number of blocks for each page)
3:OUTPUT: {𝑃𝑖𝑗} (an approximate optimal bin-packing scheme) including number of pages needed and mapping of tensor IDs and page IDs
Used in the Greedy-2 algorithm in all 4 test cases:
1. word2vec (100 * 10000, 64MB)
2. text classification (100 * 10000, 64MB)
3. text classification (300 * 300, 64MB)
4. text classification (300 * 300, 32MB)
These testing examples can be found in runGreedy-2.py in the corresponding 4 folders
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

"""
1:INPUT1: tensor_list (a set of tensors, each tensor is a set of item ids i.e. tensor blocks ids)
2:INPUT2: blocks_in_page (the maximum number of blocks for each page)
3:OUTPUT: {𝑃𝑖𝑗} (an approximate optimal bin-packing scheme) including number of pages needed
Used in the Greedy-1 algorithm in the floowing test cases:
1. text classification (100 * 10000, 64MB)
2. text classification (300 * 300, 64MB)
3. text classification (300 * 300, 32MB)
These testing examples can be found in runGreedy-1.py in the corresponding 3 folders
"""
def text_classification_greedy1(tensor_list, blocks_in_page):

    # set the required number of pages as 0
    numBins = 0

    l01234=tensor_list[0].intersection(tensor_list[1], tensor_list[2], tensor_list[3], tensor_list[4])
    p_i_j = BinPackingScheme(l01234, blocks_in_page)
    for i in range(len(l01234)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l01234) / blocks_in_page)
    p_i_j.numBins = numBins

    l0123=tensor_list[0].intersection(tensor_list[1], tensor_list[2], tensor_list[3])-l01234
    p_i_j = BinPackingScheme(l0123, blocks_in_page)
    for i in range(len(l0123)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l0123) / blocks_in_page)
    p_i_j.numBins = numBins

    l0124=tensor_list[0].intersection(tensor_list[1], tensor_list[2], tensor_list[4])-l01234
    p_i_j = BinPackingScheme(l0124, blocks_in_page)
    for i in range(len(l0124)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l0124) / blocks_in_page)
    p_i_j.numBins = numBins

    l0134=tensor_list[0].intersection(tensor_list[1], tensor_list[3], tensor_list[4])-l01234
    p_i_j = BinPackingScheme(l0134, blocks_in_page)
    for i in range(len(l0134)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l0134) / blocks_in_page)
    p_i_j.numBins = numBins

    l0234=tensor_list[0].intersection(tensor_list[2], tensor_list[3], tensor_list[4])-l01234
    p_i_j = BinPackingScheme(l0234, blocks_in_page)
    for i in range(len(l0234)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l0234) / blocks_in_page)
    p_i_j.numBins = numBins

    l1234=tensor_list[1].intersection(tensor_list[2], tensor_list[3], tensor_list[4])-l01234
    p_i_j = BinPackingScheme(l1234, blocks_in_page)
    for i in range(len(l1234)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l1234) / blocks_in_page)
    p_i_j.numBins = numBins

    l012=tensor_list[0].intersection(tensor_list[1], tensor_list[2])-l0123-l0124-l01234
    p_i_j = BinPackingScheme(l012, blocks_in_page)
    for i in range(len(l012)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l012) / blocks_in_page)
    p_i_j.numBins = numBins
    
    l013=tensor_list[0].intersection(tensor_list[1], tensor_list[3])-l0123-l0134-l01234
    p_i_j = BinPackingScheme(l013, blocks_in_page)
    for i in range(len(l013)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l013) / blocks_in_page)
    p_i_j.numBins = numBins

    l014=tensor_list[0].intersection(tensor_list[1], tensor_list[4])-l0124-l0134-l01234
    p_i_j = BinPackingScheme(l014, blocks_in_page)
    for i in range(len(l014)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l014) / blocks_in_page)
    p_i_j.numBins = numBins

    l023=tensor_list[0].intersection(tensor_list[2], tensor_list[3])-l0123-l0234-l01234
    p_i_j = BinPackingScheme(l023, blocks_in_page)
    for i in range(len(l023)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l023) / blocks_in_page)
    p_i_j.numBins = numBins
    
    l024=tensor_list[0].intersection(tensor_list[2], tensor_list[4])-l0124-l0234-l01234
    p_i_j = BinPackingScheme(l024, blocks_in_page)
    for i in range(len(l024)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l024) / blocks_in_page)
    p_i_j.numBins = numBins
    
    l034=tensor_list[0].intersection(tensor_list[3], tensor_list[4])-l0134-l0234-l01234
    p_i_j = BinPackingScheme(l034, blocks_in_page)
    for i in range(len(l034)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l034) / blocks_in_page)
    p_i_j.numBins = numBins
    
    l123=tensor_list[1].intersection(tensor_list[2], tensor_list[3])-l0123-l1234-l01234
    p_i_j = BinPackingScheme(l123, blocks_in_page)
    for i in range(len(l123)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l123) / blocks_in_page)
    p_i_j.numBins = numBins

    l124=tensor_list[1].intersection(tensor_list[2], tensor_list[4])-l0124-l1234-l01234
    p_i_j = BinPackingScheme(l124, blocks_in_page)
    for i in range(len(l124)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l124) / blocks_in_page)
    p_i_j.numBins = numBins
    
    l134=tensor_list[1].intersection(tensor_list[3], tensor_list[4])-l0134-l1234-l01234
    p_i_j = BinPackingScheme(l134, blocks_in_page)
    for i in range(len(l134)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l134) / blocks_in_page)
    p_i_j.numBins = numBins
    
    l234=tensor_list[2].intersection(tensor_list[3], tensor_list[4])-l0234-l1234-l01234
    p_i_j = BinPackingScheme(l234, blocks_in_page)
    for i in range(len(l234)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l234) / blocks_in_page)
    p_i_j.numBins = numBins
    
    l01=tensor_list[0].intersection(tensor_list[1])-l012-l013-l014-l0123-l0124-l0134-l01234
    p_i_j = BinPackingScheme(l01, blocks_in_page)
    for i in range(len(l01)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l01) / blocks_in_page)
    p_i_j.numBins = numBins

    l02=tensor_list[0].intersection(tensor_list[2])-l012-l023-l024-l0123-l0124-l0234-l01234
    p_i_j = BinPackingScheme(l02, blocks_in_page)
    for i in range(len(l02)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l02) / blocks_in_page)
    p_i_j.numBins = numBins

    l03=tensor_list[0].intersection(tensor_list[3])-l013-l023-l034-l0123-l0234-l0234-l01234
    p_i_j = BinPackingScheme(l03, blocks_in_page)
    for i in range(len(l03)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l03) / blocks_in_page)
    p_i_j.numBins = numBins

    l04=tensor_list[0].intersection(tensor_list[4])-l014-l024-l034-l0124-l0134-l0234-l01234
    p_i_j = BinPackingScheme(l04, blocks_in_page)
    for i in range(len(l04)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l04) / blocks_in_page)
    p_i_j.numBins = numBins

    l12=tensor_list[1].intersection(tensor_list[2])-l012-l123-l124-l0123-l0124-l1234-l01234
    p_i_j = BinPackingScheme(l12, blocks_in_page)
    for i in range(len(l12)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l12) / blocks_in_page)
    p_i_j.numBins = numBins

    l13=tensor_list[1].intersection(tensor_list[3])-l013-l034-l134-l0123-l0134-l1234-l01234
    p_i_j = BinPackingScheme(l13, blocks_in_page)
    for i in range(len(l13)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l13) / blocks_in_page)
    p_i_j.numBins = numBins

    l14=tensor_list[1].intersection(tensor_list[4])-l014-l124-l134-l0124-l0134-l1234-l01234
    p_i_j = BinPackingScheme(l14, blocks_in_page)
    for i in range(len(l14)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l14) / blocks_in_page)
    p_i_j.numBins = numBins

    l23=tensor_list[2].intersection(tensor_list[3])-l023-l123-l234-l0123-l0234-l1234-l01234
    p_i_j = BinPackingScheme(l23, blocks_in_page)
    for i in range(len(l23)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l23) / blocks_in_page)
    p_i_j.numBins = numBins

    l24=tensor_list[2].intersection(tensor_list[4])-l024-l124-l234-l0124-l0234-l1234-l01234
    p_i_j = BinPackingScheme(l24, blocks_in_page)
    for i in range(len(l24)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l24) / blocks_in_page)
    p_i_j.numBins = numBins

    l34=tensor_list[3].intersection(tensor_list[4])-l034-l134-l234-l0134-l0234-l1234-l01234
    p_i_j = BinPackingScheme(l34, blocks_in_page)
    for i in range(len(l34)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l34) / blocks_in_page)
    p_i_j.numBins = numBins

    l0=tensor_list[0]-l01-l02-l03-l04-l012-l013-l014-l023-l024-l034-l0123-l0124-l0134-l0234-l01234
    p_i_j = BinPackingScheme(l0, blocks_in_page)
    for i in range(len(l0)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l0) / blocks_in_page)
    p_i_j.numBins = numBins
    
    l1=tensor_list[1]-l01-l12-l13-l14-l012-l013-l014-l123-l124-l134-l0123-l0124-l0134-l1234-l01234
    p_i_j = BinPackingScheme(l1, blocks_in_page)
    for i in range(len(l1)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l1) / blocks_in_page)
    p_i_j.numBins = numBins

    l2=tensor_list[2]-l02-l12-l23-l24-l012-l023-l024-l123-l124-l234-l0123-l0124-l0234-l1234-l01234
    p_i_j = BinPackingScheme(l2, blocks_in_page)
    for i in range(len(l2)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l2) / blocks_in_page)
    p_i_j.numBins = numBins

    l3=tensor_list[3]-l03-l13-l23-l34-l013-l023-l034-l123-l134-l234-l0123-l0134-l0234-l1234-l01234
    p_i_j = BinPackingScheme(l3, blocks_in_page)
    for i in range(len(l3)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l3) / blocks_in_page)
    p_i_j.numBins = numBins

    l4=tensor_list[4]-l04-l14-l24-l34-l014-l024-l034-l124-l134-l234-l0124-l0134-l0234-l1234-l01234
    p_i_j = BinPackingScheme(l4, blocks_in_page)
    for i in range(len(l4)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l4) / blocks_in_page)
    p_i_j.numBins = numBins
    return set([p_i_j])

"""
1:INPUT1: tensor_list (a set of tensors, each tensor is a set of item ids i.e. tensor blocks ids)
2:INPUT2: blocks_in_page (the maximum number of blocks for each page)
3:OUTPUT: {𝑃𝑖𝑗} (an approximate optimal bin-packing scheme) including number of pages needed
Used in the Two-Stage algorithm in the floowing test cases:
1. text classification (100 * 10000, 64MB)
2. text classification (300 * 300, 64MB)
3. text classification (300 * 300, 32MB)
These testing examples can be found in runTwo-Stage.py in the corresponding 3 folders
"""
def text_classification_twostage(tensor_list, blocks_in_page):

    # set the required number of pages as 0
    numBins = 0

    # create a new tensor for stage-2
    new_tensor_list = [[], [], [], [], []]
    block_id = 0

    # start the first stage
    l01234=tensor_list[0].intersection(tensor_list[1], tensor_list[2], tensor_list[3], tensor_list[4])
    p_i_j = BinPackingScheme(l01234, blocks_in_page)
    newBins01234 = int(len(l01234) / blocks_in_page)
    numBins = numBins + newBins01234
    for i in range(blocks_in_page*newBins01234):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins
    this_remain = len(l01234) - blocks_in_page*newBins01234
    if (this_remain!=0):
        for i in range(block_id, block_id + this_remain):
            new_tensor_list[0].append(i)
            new_tensor_list[1].append(i)
            new_tensor_list[2].append(i)
            new_tensor_list[3].append(i)
            new_tensor_list[4].append(i)
    block_id = block_id + this_remain

    l0123=tensor_list[0].intersection(tensor_list[1], tensor_list[2], tensor_list[3])-l01234
    p_i_j = BinPackingScheme(l0123, blocks_in_page)
    newBins0123 = int(len(l0123) / blocks_in_page)
    numBins = numBins + newBins0123
    for i in range(blocks_in_page*newBins0123):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins
    this_remain = len(l0123) - blocks_in_page*newBins0123
    if (this_remain!=0):
        for i in range(block_id, block_id + this_remain):
            new_tensor_list[0].append(i)
            new_tensor_list[1].append(i)
            new_tensor_list[2].append(i)
            new_tensor_list[3].append(i)
    block_id = block_id + this_remain

    l0124=tensor_list[0].intersection(tensor_list[1], tensor_list[2], tensor_list[4])-l01234
    p_i_j = BinPackingScheme(l0124, blocks_in_page)
    newBins0124 = int(len(l0124) / blocks_in_page)
    numBins = numBins + newBins0124
    for i in range(blocks_in_page*newBins0124):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins
    this_remain = len(l0124) - blocks_in_page*newBins0124
    if (this_remain!=0):
        for i in range(block_id, block_id + this_remain):
            new_tensor_list[0].append(i)
            new_tensor_list[1].append(i)
            new_tensor_list[2].append(i)
            new_tensor_list[4].append(i)
    block_id = block_id + this_remain

    l0134=tensor_list[0].intersection(tensor_list[1], tensor_list[3], tensor_list[4])-l01234
    p_i_j = BinPackingScheme(l0134, blocks_in_page)
    newBins0134 = int(len(l0134) / blocks_in_page)
    numBins = numBins + newBins0134
    for i in range(blocks_in_page*newBins0134):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins
    this_remain = len(l0134) - blocks_in_page*newBins0134
    if (this_remain!=0):
        for i in range(block_id, block_id + this_remain):
            new_tensor_list[0].append(i)
            new_tensor_list[1].append(i)
            new_tensor_list[3].append(i)
            new_tensor_list[4].append(i)
    block_id = block_id + this_remain

    l0234=tensor_list[0].intersection(tensor_list[2], tensor_list[3], tensor_list[4])-l01234
    p_i_j = BinPackingScheme(l0234, blocks_in_page)
    newBins0234 = int(len(l0234) / blocks_in_page)
    numBins = numBins + newBins0234
    for i in range(blocks_in_page*newBins0234):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins
    this_remain = len(l0234) - blocks_in_page*newBins0234
    if (this_remain!=0):
        for i in range(block_id, block_id + this_remain):
            new_tensor_list[0].append(i)
            new_tensor_list[2].append(i)
            new_tensor_list[3].append(i)
            new_tensor_list[4].append(i)
    block_id = block_id + this_remain

    l1234=tensor_list[1].intersection(tensor_list[2], tensor_list[3], tensor_list[4])-l01234
    p_i_j = BinPackingScheme(l1234, blocks_in_page)
    newBins1234 = int(len(l1234) / blocks_in_page)
    numBins = numBins + newBins1234
    for i in range(blocks_in_page*newBins1234):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins
    this_remain = len(l1234) - blocks_in_page*newBins1234
    if (this_remain!=0):
        for i in range(block_id, block_id + this_remain):
            new_tensor_list[1].append(i)
            new_tensor_list[2].append(i)
            new_tensor_list[3].append(i)
            new_tensor_list[4].append(i)
    block_id = block_id + this_remain

    l012=tensor_list[0].intersection(tensor_list[1], tensor_list[2])-l0123-l0124-l01234
    p_i_j = BinPackingScheme(l012, blocks_in_page)
    newBins012 = int(len(l012) / blocks_in_page)
    numBins = numBins + newBins012
    for i in range(blocks_in_page*newBins012):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins
    this_remain = len(l012) - blocks_in_page*newBins012
    if (this_remain!=0):
        for i in range(block_id, block_id + this_remain):
            new_tensor_list[0].append(i)
            new_tensor_list[1].append(i)
            new_tensor_list[2].append(i)
    block_id = block_id + this_remain
    
    l013=tensor_list[0].intersection(tensor_list[1], tensor_list[3])-l0123-l0134-l01234
    p_i_j = BinPackingScheme(l013, blocks_in_page)
    newBins013 = int(len(l013) / blocks_in_page)
    numBins = numBins + newBins013
    for i in range(blocks_in_page*newBins013):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins
    this_remain = len(l013) - blocks_in_page*newBins013
    if (this_remain!=0):
        for i in range(block_id, block_id + this_remain):
            new_tensor_list[0].append(i)
            new_tensor_list[1].append(i)
            new_tensor_list[3].append(i)
    block_id = block_id + this_remain

    l014=tensor_list[0].intersection(tensor_list[1], tensor_list[4])-l0124-l0134-l01234
    p_i_j = BinPackingScheme(l014, blocks_in_page)
    newBins014 = int(len(l014) / blocks_in_page)
    numBins = numBins + newBins014
    for i in range(blocks_in_page*newBins014):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins
    this_remain = len(l014) - blocks_in_page*newBins014
    if (this_remain!=0):
        for i in range(block_id, block_id + this_remain):
            new_tensor_list[0].append(i)
            new_tensor_list[1].append(i)
            new_tensor_list[4].append(i)
    block_id = block_id + this_remain

    l023=tensor_list[0].intersection(tensor_list[2], tensor_list[3])-l0123-l0234-l01234
    p_i_j = BinPackingScheme(l023, blocks_in_page)
    newBins023 = int(len(l023) / blocks_in_page)
    numBins = numBins + newBins023
    for i in range(blocks_in_page*newBins023):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins
    this_remain = len(l023) - blocks_in_page*newBins023
    if (this_remain!=0):
        for i in range(block_id, block_id + this_remain):
            new_tensor_list[0].append(i)
            new_tensor_list[2].append(i)
            new_tensor_list[3].append(i)
    block_id = block_id + this_remain
    
    l024=tensor_list[0].intersection(tensor_list[2], tensor_list[4])-l0124-l0234-l01234
    p_i_j = BinPackingScheme(l024, blocks_in_page)
    newBins024 = int(len(l024) / blocks_in_page)
    numBins = numBins + newBins024
    for i in range(blocks_in_page*newBins024):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins
    this_remain = len(l024) - blocks_in_page*newBins024
    if (this_remain!=0):
        for i in range(block_id, block_id + this_remain):
            new_tensor_list[0].append(i)
            new_tensor_list[2].append(i)
            new_tensor_list[4].append(i)
    block_id = block_id + this_remain
    
    l034=tensor_list[0].intersection(tensor_list[3], tensor_list[4])-l0134-l0234-l01234
    p_i_j = BinPackingScheme(l034, blocks_in_page)
    newBins034 = int(len(l034) / blocks_in_page)
    numBins = numBins + newBins034
    for i in range(blocks_in_page*newBins034):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins
    this_remain = len(l034) - blocks_in_page*newBins034
    if (this_remain!=0):
        for i in range(block_id, block_id + this_remain):
            new_tensor_list[0].append(i)
            new_tensor_list[3].append(i)
            new_tensor_list[4].append(i)
    block_id = block_id + this_remain
    
    l123=tensor_list[1].intersection(tensor_list[2], tensor_list[3])-l0123-l1234-l01234
    p_i_j = BinPackingScheme(l123, blocks_in_page)
    newBins123 = int(len(l123) / blocks_in_page)
    numBins = numBins + newBins123
    for i in range(blocks_in_page*newBins123):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins
    this_remain = len(l123) - blocks_in_page*newBins123
    if (this_remain!=0):
        for i in range(block_id, block_id + this_remain):
            new_tensor_list[1].append(i)
            new_tensor_list[2].append(i)
            new_tensor_list[3].append(i)
    block_id = block_id + this_remain

    l124=tensor_list[1].intersection(tensor_list[2], tensor_list[4])-l0124-l1234-l01234
    p_i_j = BinPackingScheme(l124, blocks_in_page)
    newBins124 = int(len(l124) / blocks_in_page)
    numBins = numBins + newBins124
    for i in range(blocks_in_page*newBins124):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins
    this_remain = len(l124) - blocks_in_page*newBins124
    if (this_remain!=0):
        for i in range(block_id, block_id + this_remain):
            new_tensor_list[1].append(i)
            new_tensor_list[2].append(i)
            new_tensor_list[4].append(i)
    block_id = block_id + this_remain
    
    l134=tensor_list[1].intersection(tensor_list[3], tensor_list[4])-l0134-l1234-l01234
    p_i_j = BinPackingScheme(l134, blocks_in_page)
    newBins134 = int(len(l134) / blocks_in_page)
    numBins = numBins + newBins134
    for i in range(blocks_in_page*newBins134):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins
    this_remain = len(l134) - blocks_in_page*newBins134
    if (this_remain!=0):
        for i in range(block_id, block_id + this_remain):
            new_tensor_list[1].append(i)
            new_tensor_list[3].append(i)
            new_tensor_list[4].append(i)
    block_id = block_id + this_remain
    
    l234=tensor_list[2].intersection(tensor_list[3], tensor_list[4])-l0234-l1234-l01234
    p_i_j = BinPackingScheme(l234, blocks_in_page)
    newBins234 = int(len(l234) / blocks_in_page)
    numBins = numBins + newBins234
    for i in range(blocks_in_page*newBins234):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins
    this_remain = len(l234) - blocks_in_page*newBins234
    if (this_remain!=0):
        for i in range(block_id, block_id + this_remain):
            new_tensor_list[2].append(i)
            new_tensor_list[3].append(i)
            new_tensor_list[4].append(i)
    block_id = block_id + this_remain
    
    l01=tensor_list[0].intersection(tensor_list[1])-l012-l013-l014-l0123-l0124-l0134-l01234
    p_i_j = BinPackingScheme(l01, blocks_in_page)
    newBins01 = int(len(l01) / blocks_in_page)
    numBins = numBins + newBins01
    for i in range(blocks_in_page*newBins01):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins
    this_remain = len(l01) - blocks_in_page*newBins01
    if (this_remain!=0):
        for i in range(block_id, block_id + this_remain):
            new_tensor_list[0].append(i)
            new_tensor_list[1].append(i)
    block_id = block_id + this_remain

    l02=tensor_list[0].intersection(tensor_list[2])-l012-l023-l024-l0123-l0124-l0234-l01234
    p_i_j = BinPackingScheme(l02, blocks_in_page)
    newBins02 = int(len(l02) / blocks_in_page)
    numBins = numBins + newBins02
    for i in range(blocks_in_page*newBins02):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins
    this_remain = len(l02) - blocks_in_page*newBins02
    if (this_remain!=0):
        for i in range(block_id, block_id + this_remain):
            new_tensor_list[0].append(i)
            new_tensor_list[2].append(i)
    block_id = block_id + this_remain

    l03=tensor_list[0].intersection(tensor_list[3])-l013-l023-l034-l0123-l0234-l0234-l01234
    p_i_j = BinPackingScheme(l03, blocks_in_page)
    newBins03 = int(len(l03) / blocks_in_page)
    numBins = numBins + newBins03
    for i in range(blocks_in_page*newBins03):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins
    this_remain = len(l03) - blocks_in_page*newBins03
    if (this_remain!=0):
        for i in range(block_id, block_id + this_remain):
            new_tensor_list[0].append(i)
            new_tensor_list[3].append(i)
    block_id = block_id + this_remain

    l04=tensor_list[0].intersection(tensor_list[4])-l014-l024-l034-l0124-l0134-l0234-l01234
    p_i_j = BinPackingScheme(l04, blocks_in_page)
    newBins04 = int(len(l04) / blocks_in_page)
    numBins = numBins + newBins04
    for i in range(blocks_in_page*newBins04):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins
    this_remain = len(l04) - blocks_in_page*newBins04
    if (this_remain!=0):
        for i in range(block_id, block_id + this_remain):
            new_tensor_list[0].append(i)
            new_tensor_list[4].append(i)
    block_id = block_id + this_remain

    l12=tensor_list[1].intersection(tensor_list[2])-l012-l123-l124-l0123-l0124-l1234-l01234
    p_i_j = BinPackingScheme(l12, blocks_in_page)
    newBins12 = int(len(l12) / blocks_in_page)
    numBins = numBins + newBins12
    for i in range(blocks_in_page*newBins12):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins
    this_remain = len(l12) - blocks_in_page*newBins12
    if (this_remain!=0):
        for i in range(block_id, block_id + this_remain):
            new_tensor_list[1].append(i)
            new_tensor_list[2].append(i)
    block_id = block_id + this_remain

    l13=tensor_list[1].intersection(tensor_list[3])-l013-l034-l134-l0123-l0134-l1234-l01234
    p_i_j = BinPackingScheme(l13, blocks_in_page)
    newBins13 = int(len(l13) / blocks_in_page)
    numBins = numBins + newBins13
    for i in range(blocks_in_page*newBins13):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins
    this_remain = len(l13) - blocks_in_page*newBins13
    if (this_remain!=0):
        for i in range(block_id, block_id + this_remain):
            new_tensor_list[1].append(i)
            new_tensor_list[3].append(i)
    block_id = block_id + this_remain

    l14=tensor_list[1].intersection(tensor_list[4])-l014-l124-l134-l0124-l0134-l1234-l01234
    p_i_j = BinPackingScheme(l14, blocks_in_page)
    newBins14 = int(len(l14) / blocks_in_page)
    numBins = numBins + newBins14
    for i in range(blocks_in_page*newBins14):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins
    this_remain = len(l14) - blocks_in_page*newBins14
    if (this_remain!=0):
        for i in range(block_id, block_id + this_remain):
            new_tensor_list[1].append(i)
            new_tensor_list[4].append(i)
    block_id = block_id + this_remain

    l23=tensor_list[2].intersection(tensor_list[3])-l023-l123-l234-l0123-l0234-l1234-l01234
    p_i_j = BinPackingScheme(l23, blocks_in_page)
    newBins23 = int(len(l23) / blocks_in_page)
    numBins = numBins + newBins23
    for i in range(blocks_in_page*newBins23):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins
    this_remain = len(l23) - blocks_in_page*newBins23
    if (this_remain!=0):
        for i in range(block_id, block_id + this_remain):
            new_tensor_list[2].append(i)
            new_tensor_list[3].append(i)
    block_id = block_id + this_remain

    l24=tensor_list[2].intersection(tensor_list[4])-l024-l124-l234-l0124-l0234-l1234-l01234
    p_i_j = BinPackingScheme(l24, blocks_in_page)
    newBins24 = int(len(l24) / blocks_in_page)
    numBins = numBins + newBins01
    for i in range(blocks_in_page*newBins24):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins
    this_remain = len(l24) - blocks_in_page*newBins24
    if (this_remain!=0):
        for i in range(block_id, block_id + this_remain):
            new_tensor_list[2].append(i)
            new_tensor_list[4].append(i)
    block_id = block_id + this_remain

    l34=tensor_list[3].intersection(tensor_list[4])-l034-l134-l234-l0134-l0234-l1234-l01234
    p_i_j = BinPackingScheme(l34, blocks_in_page)
    newBins34 = int(len(l34) / blocks_in_page)
    numBins = numBins + newBins34
    for i in range(blocks_in_page*newBins34):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins
    this_remain = len(l34) - blocks_in_page*newBins34
    if (this_remain!=0):
        for i in range(block_id, block_id + this_remain):
            new_tensor_list[3].append(i)
            new_tensor_list[4].append(i)
    block_id = block_id + this_remain

    l0=tensor_list[0]-l01-l02-l03-l04-l012-l013-l014-l023-l024-l034-l0123-l0124-l0134-l0234-l01234
    p_i_j = BinPackingScheme(l0, blocks_in_page)
    newBins0 = int(len(l0) / blocks_in_page)
    numBins = numBins + newBins0
    for i in range(blocks_in_page*newBins0):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins
    this_remain = len(l0) - blocks_in_page*newBins0
    if (this_remain!=0):
        for i in range(block_id, block_id + this_remain):
            new_tensor_list[0].append(i)
    block_id = block_id + this_remain
    
    l1=tensor_list[1]-l01-l12-l13-l14-l012-l013-l014-l123-l124-l134-l0123-l0124-l0134-l1234-l01234
    p_i_j = BinPackingScheme(l1, blocks_in_page)
    newBins1 = int(len(l1) / blocks_in_page)
    numBins = numBins + newBins1
    for i in range(blocks_in_page*newBins1):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins
    this_remain = len(l1) - blocks_in_page*newBins1
    if (this_remain!=0):
        for i in range(block_id, block_id + this_remain):
            new_tensor_list[1].append(i)
    block_id = block_id + this_remain

    l2=tensor_list[2]-l02-l12-l23-l24-l012-l023-l024-l123-l124-l234-l0123-l0124-l0234-l1234-l01234
    p_i_j = BinPackingScheme(l2, blocks_in_page)
    newBins2 = int(len(l2) / blocks_in_page)
    numBins = numBins + newBins2
    for i in range(blocks_in_page*newBins2):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins
    this_remain = len(l2) - blocks_in_page*newBins2
    if (this_remain!=0):
        for i in range(block_id, block_id + this_remain):
            new_tensor_list[2].append(i)
    block_id = block_id + this_remain

    l3=tensor_list[3]-l03-l13-l23-l34-l013-l023-l034-l123-l134-l234-l0123-l0134-l0234-l1234-l01234
    p_i_j = BinPackingScheme(l3, blocks_in_page)
    newBins3 = int(len(l3) / blocks_in_page)
    numBins = numBins + newBins3
    for i in range(blocks_in_page*newBins3):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins
    this_remain = len(l3) - blocks_in_page*newBins3
    if (this_remain!=0):
        for i in range(block_id, block_id + this_remain):
            new_tensor_list[3].append(i)
    block_id = block_id + this_remain

    l4=tensor_list[4]-l04-l14-l24-l34-l014-l024-l034-l124-l134-l234-l0124-l0134-l0234-l1234-l01234
    p_i_j = BinPackingScheme(l4, blocks_in_page)
    newBins4 = int(len(l4) / blocks_in_page)
    numBins = numBins + newBins4
    for i in range(blocks_in_page*newBins4):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins
    this_remain = len(l4) - blocks_in_page*newBins4
    if (this_remain!=0):
        for i in range(block_id, block_id + this_remain):
            new_tensor_list[4].append(i)
    block_id = block_id + this_remain
    
    # start the second stage
    P = set()
    P, tensor_page_mapping = bin_pack_greedy(new_tensor_list, blocks_in_page)
    L = list(P)
    L[0].numBins = numBins + L[0].numBins
    P = set(L)
    P.union(set([p_i_j]))
    return P

"""
1:INPUT1: T (a set of tensors, each tensor is a set of item ids i.e. tensor blocks ids)
2:INPUT2: l (the maximum number of blocks for each page)
3:OUTPUT: {𝑃𝑖𝑗} (an approximate optimal bin-packing scheme) including number of pages needed
Used in the Greedy-1 algorithm in the following test:
1. word2vec (100 * 10000, 64MB)
This testing example can be found in runGreedy-1.py in the corresponding folder
"""
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
    p_i_j.numBins = numBins

    l0=tensor_list[0]-l
    p_i_j = BinPackingScheme(l0, blocks_in_page)
    for i in range(len(l0)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l0) / blocks_in_page)
    p_i_j.numBins = numBins

    l1=tensor_list[1]-l
    p_i_j = BinPackingScheme(l1, blocks_in_page)
    for i in range(len(l1)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l1) / blocks_in_page)
    p_i_j.numBins = numBins

    l2=tensor_list[2]-l
    p_i_j = BinPackingScheme(l2, blocks_in_page)
    for i in range(len(l2)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l2) / blocks_in_page)
    p_i_j.numBins = numBins

    l3=tensor_list[3]-l
    p_i_j = BinPackingScheme(l3, blocks_in_page)
    for i in range(len(l3)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l3) / blocks_in_page)
    p_i_j.numBins = numBins

    l4=tensor_list[4]-l
    p_i_j = BinPackingScheme(l4, blocks_in_page)
    for i in range(len(l4)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l4) / blocks_in_page)
    p_i_j.numBins = numBins

    l5=tensor_list[5]-l
    p_i_j = BinPackingScheme(l5, blocks_in_page)
    for i in range(len(l5)):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    numBins = numBins + math.ceil(len(l5) / blocks_in_page)
    p_i_j.numBins = numBins

    return set([p_i_j])

"""
1:INPUT1: T (a set of tensors, each tensor is a set of item ids i.e. tensor blocks ids)
2:INPUT2: l (the maximum number of blocks for each page)
3:OUTPUT: {𝑃𝑖𝑗} (an approximate optimal bin-packing scheme) including number of pages needed
Used in the Two-Stage algorithm in the following test:
1. word2vec (100 * 10000, 64MB)
This testing example can be found in runTwo-Stage.py in the corresponding folder
"""
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
    p_i_j.numBins = numBins

    l0=tensor_list[0]-l
    p_i_j = BinPackingScheme(l0, blocks_in_page)
    newBins0 = int(len(l0) / blocks_in_page)
    numBins = numBins + newBins0
    for i in range(blocks_in_page*newBins0):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins

    l1=tensor_list[1]-l
    p_i_j = BinPackingScheme(l1, blocks_in_page)
    newBins1 = int(len(l1) / blocks_in_page)
    numBins = numBins + newBins1
    for i in range(blocks_in_page*newBins1):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins

    l2=tensor_list[2]-l
    p_i_j = BinPackingScheme(l2, blocks_in_page)
    newBins2 = int(len(l2) / blocks_in_page)
    numBins = numBins + newBins2
    for i in range(blocks_in_page*newBins2):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins

    l3=tensor_list[3]-l
    p_i_j = BinPackingScheme(l3, blocks_in_page)
    newBins3 = int(len(l3) / blocks_in_page)
    numBins = numBins + newBins3
    for i in range(blocks_in_page*newBins3):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins

    l4=tensor_list[4]-l
    p_i_j = BinPackingScheme(l4, blocks_in_page)
    newBins4 = int(len(l4) / blocks_in_page)
    numBins = numBins + newBins4
    for i in range(blocks_in_page*newBins4):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins

    l5=tensor_list[5]-l
    p_i_j = BinPackingScheme(l5, blocks_in_page)
    newBins5 = int(len(l5) / blocks_in_page)
    numBins = numBins + newBins5
    for i in range(blocks_in_page*newBins5):
        j = i + 1
        s = math.ceil(i / blocks_in_page) + 1
        p_i_j.mark(j, s)
    p_i_j.numBins = numBins

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
    L[0].numBins = numBins + L[0].numBins
    P = set(L)
    P.union(set([p_i_j]))
    return P

"""
1:INPUT1: T (a set of tensors, each tensor is a set of item ids i.e. tensor blocks ids)
2:INPUT2: l (the maximum number of blocks for each page)
3:OUTPUT: {𝑃𝑖𝑗} (an approximate optimal bin-packing scheme) including number of pages needed
Used in the Baseline algorithm in all 4 test cases:
1. word2vec (100 * 10000, 64MB)
2. text classification (100 * 10000, 64MB)
3. text classification (300 * 300, 64MB)
4. text classification (300 * 300, 32MB)
These testing examples can be found in runBaseline.py in the corresponding 4 folders
"""
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