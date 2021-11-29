import numpy as np

def compute_3q(arr):
    """Compute 3rd quartile value for given matrix

    Args:
        arr (numpy array): a numpy array
    
    Return:
        float: 3rd quatile value
    """
    val_3q = np.percentile(arr, 75)
    return val_3q

def no_magnitude(arr):
    """Return 0 as magnitude for a given matrix, which is used for baseline
    without magnitude ordering

    Args:
        arr (numpy array): a numpy array
    
    Return:
        0
    """
    return 0

def pad_arr_2d(weight_idx, arr_2d, block_size_x, block_size_y=None, compute_magnitude=compute_3q):
    """Pad a 2d array to be able to be blocked with given block size. The magnitude
    of a block is computed before padding

    Args: 
        weight_idx (int): index of the array in the weights list
        arr_2d (numpy array): 2d array that needs to be padded
        block_size_x (int): width of a block
        block_size_y (int): height of a block. If not speicifed, it will be set as 
                            block_size_x, default=None
        compute_magnitude (function): the way to compute the magnitude of a block, 
                            defaule=compute a block's 3rd quartile value
        
    Return:
        numpy array: padded numpy array
        dict: a dictionary to store the block's magnitude, it is organized as 
              (weight_idx, i, j): magnitude value pair.
        int: number of blocks in x-axis
        int: number of blocks in y-axis
    """
    assert len(arr_2d.shape) == 2

    if block_size_y is None:
        block_size_y = block_size_x
    num_x, num_y = arr_2d.shape
    num_block_x = int(np.ceil(num_x / block_size_x))
    num_block_y = int(np.ceil(num_y / block_size_y))

    blocks_magnitude = {}
    for i in range(num_block_x):
        for j in range(num_block_y):
            block = arr_2d[i*block_size_x: (i+1)*block_size_x,
                           j*block_size_y: (j+1)*block_size_y]
            blocks_magnitude[(weight_idx, i, j)] = compute_magnitude(block)
    arr_2d_padded = np.pad(arr_2d, [(0, num_block_x*block_size_x - num_x),
                           (0, num_block_y*block_size_y - num_y)], mode='constant', constant_values=0)
    return arr_2d_padded, blocks_magnitude, num_block_x, num_block_y


def block_model_2d(model, block_size_x, block_size_y=None, compute_magnitude=compute_3q):
    """Block a keras model

    Args:
        model (tf.keras.Models): the model to be blocked
        block_size_x (int): width of a block
        block_size_y (int): height of a block. If not speicifed, it will be set as 
                            block_size_x, default=None
        compute_magnitude (function): the way to compute the magnitude of a block, 
                            defaule=compute a block's 3rd quartile value
    
    Return:
        dict: a dictionary object to store the data of blocked model, called model storage.
              it has four keys, weights_padded, weights_shape, weights_block_num, block_magnitude.
              'weights_padded' stores the padded blocks. 'weights_shape' stores the shape of the 
              original weight matrix. 'weights_block_num' stores the number of blocks in both 
              x-axis and y-axis after padding. 'block_magnitude' is a dictionary that stores 
              each block's magnitude as key-value pairs.

    """
    if block_size_y is None:
        block_size_y = block_size_x
    model_storage = dict()
    weights = model.get_weights().copy()
    weights_discritized = weights.copy()
    block_magnitude = {}
    weights_shape = []
    weights_block_num = []

    discrete_bins = np.linspace(-1, 1, 10000)

    
    for idx in range(len(weights)):
        w = weights[idx]
        w_shape = w.shape

        # if it is bias (1-d) arr, add 1 dimension to convert it to 2-d 
        if len(w_shape) == 1:
            w = w.reshape(-1, 1)
        w_padded, w_magnitude, w_num_block_x, w_num_block_y = pad_arr_2d(idx, w,
                                                                         block_size_x=block_size_x, block_size_y=block_size_y, compute_magnitude=compute_magnitude)
        w_padded_discritized = np.digitize(w_padded, discrete_bins)
        weights[idx] = w_padded
        weights_discritized[idx] = w_padded_discritized
        block_magnitude.update(w_magnitude)
        weights_shape.append(w_shape)
        weights_block_num.append((w_num_block_x, w_num_block_y))

    model_storage['weights_padded'] = weights
    model_storage['weights_padded_discritized'] = weights_discritized
    model_storage['weights_shape'] = weights_shape
    model_storage['weights_block_num'] = weights_block_num
    model_storage['block_magnitude'] = block_magnitude
    return model_storage


def reconstruct_weight_2d(model_storage):
    """Reconstruct a model weights from a given model storage object

    Args:
        model_storage (model_storage): a model's model_storage 
    
    Return:
        numpy array: reconstructed weights
    """
    weights_reconstructed = model_storage['weights_padded'].copy()
    for idx in range(len(weights_reconstructed)):
        w = weights_reconstructed[idx]
        w_shape = model_storage['weights_shape'][idx]
        if len(w_shape) == 1:
            w = w[:w_shape[0], 0]
        else:
            w = w[:w_shape[0], :w_shape[1]]
        weights_reconstructed[idx] = w

    return weights_reconstructed