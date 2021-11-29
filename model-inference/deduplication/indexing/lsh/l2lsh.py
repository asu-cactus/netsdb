import random
import numpy as np
from collections import defaultdict


class L2LSH(object):
    """L2LSH class, each hash value is computed by np.ceil((a @ X + b) / r)

    Args:
        prob_dim (int): size of probability vector
        r (float, optional): r is a real positive number
        num_l (int, optional): number of band,
            default=40
        num_k (int, optional): number of concatenated hash function, 
            default=4
        seed (int, optional): random seed that is used to generate hash
            function coefficients, default=1

    """

    def __init__(self, prob_dim, r=8, num_l=40, num_k=4, seed=None):
        """Initialize the L2LSH.

        Args:
            prob_dim (int): size of probability vector
            r (float, optional): a real positive number, default=1
            num_l (int, optional): number of band,
            default=40
        num_k (int, optional): number of concatenated hash function, 
            default=4
        seed (int, optional): random seed that is used to generate hash
            function coefficients, default=1

        """

        if seed != None:
            self.seed = seed
            np.random.seed(seed)

        self.num_l = num_l
        self.num_k = num_k
        self.r = r

        # Initiate the coefficients, num_l*num_k hash functions in total
        self.a = np.random.normal(size=(num_l*num_k, prob_dim))
        # b is uniformly at random on [0, r]
        self.b = np.random.uniform(low=0, high=self.r, size=(num_l*num_k, 1))

        # generate the hash ranges for each band
        self.hash_ranges = [(i*self.num_k, (i+1)*self.num_k)
                            for i in range(self.num_l)]
        self.hash_table = defaultdict(set)

    def _compute_concatenated_lsh(self, hash_matrix):
        """Compute concatenated hash value for each band

        Args:
            hash_matrix (array): a hash value vector
        
        Return:
            int: concatenated hash value
        """

        str_value = [str(x) for x in hash_matrix]
        concatenated_hash_value = ''.join(str_value)
        
        return concatenated_hash_value

    def compute_lsh(self, prob):
        """Compute hash value for the given probability vector

        Args:
            prob (array): a square root probability vector
        """
        return np.ceil((self.a @ prob + self.b) / self.r)


    def insert(self, prob, key):
        """Insert a probability vector with key name and store them in hash_table

        Args:
            prob (array): a square root probability vector
            key (string): key name of the inserted probability vector 
        """
        # Reshape from (dim, ) to (dim, 1) to avoid Python broadcasting error
        prob = np.reshape(prob, (-1,1))

        lsh_values = self.compute_lsh(prob).flatten()

        # Insert each band hash value into hash table
        index = 0

        # Concatenation AND
        for start, end in self.hash_ranges:
            
            concatenated_hash_value = self._compute_concatenated_lsh(lsh_values[start: end])
            
            dict_key = ''.join(['L', str(index), '_', concatenated_hash_value])     

            self._insert(dict_key, key)
            
            index += 1
    
    def query(self, prob, threshold=0):
        """Retrieve the keys of probs that are similiar to the 
        given probability vector

        Args:
            prob (array): a square root probability vector
            threshold (int, optional): number of collided hash value

        """
        # Reshape from (dim, ) to (dim, 1) to avoid Python broadcasting error
        prob = np.reshape(prob, (-1,1))

        # Compute LSH for the given probability vector
        candidates = set()
        lsh_values = self.compute_lsh(prob).flatten()

        # Construct a hit dictonary
        candidate_hit = defaultdict(int)
        
        
        # Retrieve the probs
        index = 0      
        for start, end in self.hash_ranges:

            concatenated_hash_value = self._compute_concatenated_lsh(lsh_values[start: end])
            
            dict_key = ''.join(['L', str(index), '_', concatenated_hash_value])     

            for val in self.hash_table.get(dict_key, []):
                candidate_hit[val] += 1
            
            index += 1
        
        for k, v in candidate_hit.items():
            if v >= threshold:
                candidates.add(k)
        
        return list(candidates)
    
    def _insert(self, key, value):
        """Insert a band hash value to the hash table

        Args:
            key (int): band hash value
            value (str): key name of the given probability vector

        """
        self.hash_table[key].add(value)