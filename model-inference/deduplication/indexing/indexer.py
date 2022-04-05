import numpy as np
from tqdm import tqdm


class Indexer(object):
    """Class to build index and store blocks
    """

    def __init__(self, block_size_x, block_size_y=None):
        """Initialization, if block_size_y is not specified, it will be set as block_size_x. 

        Args:
            block_size_x (int): width of a block
            block_size_y (int): height of a block. If not speicifed, it will be set as 
                                block_size_x, default=None
        """
        self.block_size_x = block_size_x
        self.block_size_y = block_size_y
        # block capacitiy
        self.block_cap = block_size_x*block_size_y
        # list to store blocks
        self.list_blocks = []
        # number of blocks get stored
        self.num_total = 0
        # set to store block names
        self.model_names = set()
        self.list_blocks_discritized = []
    
    def discritized(self):
        """Discritized function which is used by approximate match
        """
        discrete_bins = np.linspace(-1, 1, 10000)
        for i in tqdm(range(self.num_total)):
            b = self.list_blocks[i]
            b_discritized = np.digitize(b, discrete_bins)
            self.list_blocks_discritized.append(b_discritized)


    def save(self, path='indexer.npy'):
        """Save indexer data to disk.

        Args:
            path (string): path to save indexer data.
        """
        indexer_data = dict()
        indexer_data['block_size_x'] = self.block_size_x
        indexer_data['block_size_y'] = self.block_size_y
        indexer_data['block_cap'] = self.block_cap
        indexer_data['list_blocks'] = self.list_blocks
        indexer_data['num_total'] = self.num_total
        indexer_data['model_names'] = self.model_names
        np.save(path, indexer_data)
        print('indexer data saved successfully. Save to :', path)
    
    def load(self, path='indexer.npy'):
        """Load indexer data from disk

        Args:
            path (string): path to load indexer data
        """
        indexer_data = np.load(path, allow_pickle=True).item()
        self.block_size_x = indexer_data['block_size_x']
        self.block_size_y = indexer_data['block_size_y']
        self.block_cap = indexer_data['block_cap']
        self.list_blocks = indexer_data['list_blocks']
        self.num_total = indexer_data['num_total']
        self.model_names = indexer_data['model_names'] 
        print('load indexer data successfully. Load from :', path)
        
    def build_index(self, model_storage, model_name):
        """Build index with given model_storage

        Args: 
            model_storage (model_storage): output of blocking a model.
            model_name (string): name of the model
        
        Return:
            dict: a dictionary to store the mapping of (weight_idx, block_idx) -> block index 
                  in the Indexer
        
        """
        if model_name in self.model_names:
            raise ValueError('indexer already exist')
        else:
            self.model_names.add(model_name)
        # index_finder is a dictionary to store the mapping of 
        # (weight_idx, block_idx) -> block index in the Indexer, which is used to avoid
        # self deduplication.
        index_finder = dict()
        for m_weight_idx in range(len(model_storage['weights_padded'])):
            # 2D blocking
            m_weight_block_x_num, m_weight_block_y_num = model_storage[
                'weights_block_num'][m_weight_idx]
            # FIXME faster inference ? 1 loop use val // y val % y to get x and y
            for i in range(m_weight_block_x_num):
                for j in range(m_weight_block_y_num):
                    b_identifier = ''.join(
                        [str(m_weight_idx), '_', str(i), '_', str(j)])
                    b = model_storage['weights_padded'][m_weight_idx][i*self.block_size_x: (
                        i+1)*self.block_size_x, j*self.block_size_y: (j+1)*self.block_size_y]
                    self.list_blocks.append(b)
                    index_finder[b_identifier] = self.num_total
                    self.num_total += 1
        return index_finder
    
    def update_index(self, model_storage, model_dedup_output):
        """Build index for the remaining blocks, which are not deduplicated.

        Args:
            model_storage (model_storage): output of blocking a model
            model_dedup_output (dataframe): output of deduplication detector
        """
        df_filter_by_dedup = model_dedup_output[model_dedup_output['is_deduplicated'] == True]
        list_duplicate_block_idx = df_filter_by_dedup['duplicate_block_idx'].to_list()
        list_duplicate_block_idx = [str(val) for val in list_duplicate_block_idx]
        
        count = 0
        for m_weight_idx in range(len(model_storage['weights_padded'])):
            # 2D blocking
            m_weight_block_x_num, m_weight_block_y_num = model_storage['weights_block_num'][m_weight_idx]
            
            for i in range(m_weight_block_x_num):
                for j in range(m_weight_block_y_num):
                    b_identifier = ''.join(
                        [str(m_weight_idx), '_', str(i), '_', str(j)])
                    b_index = (m_weight_idx, i, j)
                    if str(b_index) not in list_duplicate_block_idx:
                        b = model_storage['weights_padded'][m_weight_idx][i*self.block_size_x: (
                        i+1)*self.block_size_x, j*self.block_size_y: (j+1)*self.block_size_y]
                        self.list_blocks.append(b)
                        self.num_total += 1
                        count += 1
                        print('{} blocks added'.format(b_index))
        print('Total: {} blocks added'.format(count))