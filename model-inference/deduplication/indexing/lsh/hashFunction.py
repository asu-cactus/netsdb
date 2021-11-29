import hashlib

def hash_sha1(input):
    """A hash function based hashlib.sha1
    
    Args:
        input: the data to generate integer-based hash value (need to be encoded with UTF-8)
    
    Returns:
        int: an integer-based hasah values
    """

    if (isinstance(input, bytes) == False):
        raise TypeError("The input data must be encoded with UTF-8")
    
    return int(hashlib.sha1(input).hexdigest()[0:8], 16)
