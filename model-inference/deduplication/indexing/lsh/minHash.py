import numpy as np
import random
from hashFunction import hash_sha1

# this code is modified from
# https://github.com/ekzhu/datasketch/blob/master/datasketch/minhash.py

# Mersenne prime
MERSENNE_PRIME = (1 << 31) - 1
MAX_PERMU = (1 << 31)
MAX_HASH = (1 << 31)
RANDOM_SEED = 1 

class MinHash(object):
    """MinHash is a convenient class to estimate 'Jaccard Similarity' 
    between sets based on MinHash.

    Args:
        num_permu (int, optional): number of permutation functions,
            default=128
        seed (int, optional): the random seed that is used to generate
            random permutation function, default=1
        hash_func (function, optional): the hash function used to generate
            hash value, default=hash_sha1
    
    Methods:
        add_and_update(item)
        jaccard((MinHash)another)
    
    """

    def __init__(self, num_permu=128, seed=1, hash_func=hash_sha1):
        self.seed = seed
        random.seed(seed)
        
        # Check number of permutation range
        if num_permu > MAX_PERMU:
            raise ValueError("The number of permutation cannot exceed %d" % MAX_PERMU)
        self.num_permu = num_permu
        
        # Check the hash function is callable
        if not callable(hash_func):
            raise ValueError("Please provide a valid hash function")
        self.hash_func = hash_func
        
        # Initiate the coefficient for each permutation
        # Here use formula: y = (a*X + b) mod MERSENNE_PRIME to generate the signature. Each a and b are sampled without replacement.
        # The size of permutations_coef: [2, num_permu]
        
        self.permutations_coef = np.array([random.sample(range(1, MERSENNE_PRIME), num_permu),
                                           random.sample(range(0, MERSENNE_PRIME), num_permu)], dtype=np.uint64)
        
        # Initiate the hash matrix to store the minimum hash value for this MinHash object
        # The size of hash_matrix: [num_permu, 1]
        self.hash_matrix = np.ones(num_permu, dtype=np.uint64) * MAX_HASH
    
    def add_and_update(self, item):
        """Add a new item and update MinHash.

        Args:
            item: new item (should be encoded with UTF-8 first)

        Example:

        """
        hashed_value = self.hash_func(item)
        a, b = self.permutations_coef
        result = (a * hashed_value + b) % MERSENNE_PRIME
        self.hash_matrix = np.minimum(self.hash_matrix, result)

    def add_a_matrix(self, matrix):
        """Vectorized implementation to create a MinHash for a numpy array
        """
        get_hash_lambda = lambda t: hash(t)
        func_hash = np.vectorize(get_hash_lambda)
        hashed_values = func_hash(matrix).reshape(-1,1)
        a, b = self.permutations_coef
        result = (hashed_values @ a.reshape(1,-1) + b.reshape(1,-1)) % MERSENNE_PRIME
        self.hash_matrix = np.min(result, axis=0)

    def jaccard(self, another):
        """Estimate 'Jaccard Similarity' based on MinHash between this object and another object

        Args:
            another: another object

        Returns:
            floats: an estimated jaccard similarity, which is between 0.0 and 1.0

        """
        if self.num_permu != another.num_permu:
            raise ValueError("Cannot compute jaccard similarity if the given objects come with different numbers of permutation")
        if self.seed != another.seed:
            raise ValueError("Cannot compute jaccard similarity if the given objects come with different random seed")

        return np.float(np.count_nonzero(self.hash_matrix == another.hash_matrix) / self.num_permu)