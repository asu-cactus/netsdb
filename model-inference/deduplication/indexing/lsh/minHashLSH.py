import random
import numpy as np
import scipy.integrate as integrate
from collections import defaultdict
from hashFunction import hash_sha1

# this code is modified from
# https://github.com/ekzhu/datasketch/blob/master/datasketch/lsh.py

# Mersenne prime
MERSENNE_PRIME = (1 << 31) - 1
MAX_PERMU = (1 << 31)
MAX_HASH = (1 << 31)
RANDOM_SEED = 1 

def _false_positive(num_b, num_r, threshold):
    """Calculate the false positive probability based on the probability distribution
    p = 1 - (1 - threshold**num_r)**num_b

    Args:
        num_b (int): number of bands
        num_r (int): number of rows of each band
        threshold (float): jaccard similarity threshold that the datasets 
            can be assumed similar
    Returns:
        float: the integral of the given function
    """
    f = lambda t: (1 - (1 - t**num_r)**num_b)
    # Return integral 
    return integrate.quad(f, 0, threshold)[0] 

def _false_negative(num_b, num_r, threshold):
    """Calculate the false negative probability based on the probability distribution
    p = 1 - (1 - threshold**num_r)**num_b

    Args:
        num_b (int): number of bands
        num_r (int): number of rows of each band
        threshold (float): jaccard similarity threshold that the datasets 
            can be assumed similar, the range is between 0.0 and 1.0
    Returns:
        float: the integral of the given function
    """
    f = lambda t: (1 - (1 - (1 - t**num_r)**num_b))
    # Return integral 
    return integrate.quad(f, threshold, 1)[0] 

def _init_params(num_permu, threshold):
    """Initiate and optimize the num_b and num_r based on given num_permu and 
    threshold

    Args:
        num_permu (int): number of permutations
        threshold (float): jaccard similarity threshold that the datasets 
            can be assumed similar, the range is between 0.0 and 1.0
    Returns:
        (int, int): return the optimized (num_b, num_r)
    """

    min_cost = float("inf")
    optimized_params = (0, 0)

    for num_b in range(1, num_permu+1):
        max_num_r = int(num_permu / num_b)
        for num_r in range(1, max_num_r+1):
            false_positive = _false_positive(num_b, num_r, threshold)
            false_negative = _false_negative(num_b, num_r, threshold)
            cost = 0.5*false_positive + 0.5*false_negative        
            if cost < min_cost:
                min_cost = cost
                optimized_params = (num_b, num_r)

    return optimized_params

class MinHashLSH(object):
    """MinHashLSH is a convenient class based on 'Locality-Sensitive Hashing' 
    to find a subset of datas which may joinable to given query. The returned
    data's jaccard similarity needs to greater than a given threshold.
    
    Args:
        threshold (float, optional): the range of the threshold is between 0.0 and 1.0,
            default=0.5
        num_permu (int, optional): the number of permutation functions
        num_b (int, optional): the number of bands. If num_b and num_r are not provided, 
            these values will be initiated with an optimal function
        num_r (int, optional): the number of rows, a restriction of num_permu, num_b, and num_r:
            num_b * num_r <= num_permu
        seed (int, optional): the random seed that is used to generate
            random permutation function, default=1
        hash_func (function, optional): the hash function used to generate
            hash value, default=hash_sha1
        duplicate_key (boolean, optional): a flag that allows duplicate key when inserting, 
            default=False
    
    """
    
    def __init__(self, threshold=0.5, num_permu=128, num_b=None, num_r=None, seed=1, hash_func=hash_sha1, duplicate_key=False):
        self.seed = seed
        self.duplicate_key = duplicate_key
        random.seed(seed)
        
        # Validate the number of permutation range
        if num_permu > MAX_PERMU:
            raise ValueError("The number of permutation cannot exceed %d" % MAX_PERMU)
        self.num_permu = num_permu
        
        # Validate the hash function is callable
        if not callable(hash_func):
            raise ValueError("Please provide a valid hash function")
        self.hash_func = hash_func
        
        # Validate num_b and num_r 
        if num_b is not None and num_r is not None:
            if num_b * num_r > num_permu:
                # restriction of num_b, num_r, and num_permu does not meet
                raise ValueError("The product of num_b and num_r is not valid:" 
                                 " {} * {} = {}. It should be not greater than {}."
                                 .format(num_b, num_r, num_b * num_r, num_permu))
            else:
                self.num_b = num_b
                self.num_r = num_r
        else:
             # num_b or num_r missing, call automatic optimized function to init
            self.num_b, self.num_r = _init_params(num_permu, threshold)    
        
        # Initiate storage for keys and hash table
        self.hash_ranges = [(i*self.num_r, (i+1)*self.num_r) for i in range(self.num_b)]
        self.hash_table = defaultdict(set)
        self.keys = set()
        
        # Use formula y = a*X + b 
        self.a = random.randint(1, 10)
        self.b = random.randint(0, 10)
    
    
    def insert(self, key, minhash):
        """Insert a MinHash object with key name and store them in hash_table and keys

        Args:
            key (str): key name of the MinHash object
            minhash (MinHash): a MinHash object
        """
        # Validate num_permu
        if self.num_permu != minhash.num_permu:
            raise ValueError("The given minhash's length does not match the LSH's."
                             " Expect {}, but got {}".format(self.num_permu, minhash.num_permu))
        # Check duplication
        if not self.duplicate_key and key in self.keys:
            raise ValueError("The given key {} already exist.".format(key))
        
        # Insert the new key name into self.keys
        self.keys.add(key)
        
        # Generate the signature value for each band with given MinHash
        lsh_value = [self._compute_lsh(minhash.hash_matrix[start: end])
                    for start, end in self.hash_ranges]
        

        # Insert the given MinHash's band signature into corresponding bucket
        # The data will be stored as {band_hash: key_name}
        for band_hash in lsh_value:
            self._insert(band_hash, key)
            
    def query(self, minhash):
        """Retrieve the keys of sets whose jaccard similarities with given MinHash 
        are greater than the threshold.

        Args:
            minhash (MinHash): A MinHash object

        Return:
            list: A list of datasets keynames        
        """
        # Validate num_permu
        if self.num_permu != minhash.num_permu:
            raise ValueError("The given minhash's length does not match the LSH's."
                             " Expect {}, but got {}".format(self.num_permu, minhash.num_permu))
        candidates = set()
        # Compute given MinHash's bands signature
        lsh_value = [self._compute_lsh(minhash.hash_matrix[start: end])
                    for start, end in self.hash_ranges]
        # Retrieve the keynames of datasets in the same bucket
        for h in lsh_value:
            for key in self.hash_table.get(h, []):
                candidates.add(key)
        
        return list(candidates)
        
    
    def _compute_lsh(self, hash_matrix_subset):
        """Compute the hash value of a band of whole hash_matrix

        Args:
            hash_matrix_subset (array-like of shape)
        
        Returns:
            int: hashed value
        """
        return np.sum(self.a * hash_matrix_subset + self.b) % MERSENNE_PRIME
    
    def _insert(self, band_hash, key):
        """Insert a computed band hash value to the hash table of LSH

        Args: 
            band_hash (int): computed hash value
            key (str): key name of the given MinHash object
        """
        # If not exist, init
        if band_hash not in self.hash_table.keys():
            self.hash_table[band_hash] = set()
        # Check if key is in hash_table[band_hash]]
        if key not in self.hash_table[band_hash]:
            self.hash_table[band_hash].add(key)