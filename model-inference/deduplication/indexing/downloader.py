import gdown
import os

def dataset():
    """Download preprocessed datasets.
    """
    if not os.path.isfile('model_deduplication_dataset.zip'):
        model_deduplication_dataset_url = 'https://drive.google.com/uc?id=1nYzDDSJGkjCsVQI4gbSdC3DafQ7Ez107'
        gdown.download(model_deduplication_dataset_url, output=None, quiet=False)
        os.system('unzip -qqq model_deduplication_dataset.zip  ')
    else:
        print('Dataset already downloadded')

def model(model_dir='models'):
    """Download pre-trained models.

    Args:
        model_dir: string, path to store download models
    """
    if not os.path.isdir(model_dir):
        os.makedirs(model_dir)
    # w2v_wiki500_imdb_embed_nontrainable
    if not os.path.isfile(os.path.join(model_dir, 'w2v_wiki500_imdb_embed_nontrainable.h5')):
        model_url = 'https://drive.google.com/uc?id=1jG2UjS75KG4pOeRKCWYdZd1o3TQlewfy'
        gdown.download(model_url, output=None, quiet=False)
        os.system('mv w2v_wiki500_imdb_embed_nontrainable.h5 ' + model_dir + '/')
    
    # w2v_wiki500_imdb_embed_trainable
    if not os.path.isfile(os.path.join(model_dir, 'w2v_wiki500_imdb_embed_trainable.h5')):
        model_url = 'https://drive.google.com/uc?id=1-1I6r7kBQhwyHi5_Xatrwz_VFVM3H8GH'
        gdown.download(model_url, output=None, quiet=False)
        os.system('mv w2v_wiki500_imdb_embed_trainable.h5 ' + model_dir + '/')

    # w2v_wiki500_yelp_embed_nontrainable
    if not os.path.isfile(os.path.join(model_dir, 'w2v_wiki500_yelp_embed_nontrainable.h5')):
        model_url = 'https://drive.google.com/uc?id=1-6T6c5MaaceARapMnPBEj0KP_P1-rg0y'
        gdown.download(model_url, output=None, quiet=False)
        os.system('mv w2v_wiki500_yelp_embed_nontrainable.h5 ' + model_dir + '/')

    # w2v_wiki500_yelp_embed_trainable
    if not os.path.isfile(os.path.join(model_dir, 'w2v_wiki500_yelp_embed_trainable.h5')):
        model_url = 'https://drive.google.com/uc?id=1GgVaiexh643C7LlVH0qJHqlJxBZo71M0'
        gdown.download(model_url, output=None, quiet=False)
        os.system('mv w2v_wiki500_yelp_embed_trainable.h5 ' + model_dir + '/')

    # w2v_wiki500_civil_comment_embed_trainable
    if not os.path.isfile(os.path.join(model_dir, 'w2v_wiki500_civil_comment_embed_trainable.h5')):
        model_url = 'https://drive.google.com/uc?id=1--bCnYYoe0mXseM783qqkncVMQVQmbhy'
        gdown.download(model_url, output=None, quiet=False)
        os.system('mv w2v_wiki500_civil_comment_embed_trainable.h5 ' + model_dir + '/')

def main():
    download_dataset = True
    download_model = False
    if download_dataset:
        print('Downloading pre-processed datasets')
        dataset()
        print('Download dataset: Success!')
        
    if download_model:
        print('Downloading pre-trained models')
        model()
        print('Download pre-trained models: Success!')

if __name__ == "__main__":
    main()