import gdown
import os

def dataset(task='all'):
    """Download preprocessed datasets.

    Args:
        task (str): all - download all datasets,
                    sentiment - download sentiment datasets, 
                    extremes - download extreme datasets
    """
    if task == 'all' or task == 'sentiment':
        if not os.path.isfile('model_deduplication_sentiment_dataset.zip'):
            model_deduplication_dataset_url = 'https://drive.google.com/uc?id=1nYzDDSJGkjCsVQI4gbSdC3DafQ7Ez107'
            gdown.download(model_deduplication_dataset_url, output=None, quiet=False)
            os.system('unzip -qqq model_deduplication_sentiment_dataset.zip')
        else:
            print('Dataset already downloadded')
    if task == 'all' or task == 'extremes':
        # TODO 
        if not os.path.isfile('model_deduplication_extreme_dataset.zip'):
            model_deduplication_dataset_url = 'https://drive.google.com/uc?id=1WuqGC2uPRbZsxbhK9oLjJDoJMgVtIqw8'
            gdown.download(model_deduplication_dataset_url, output=None, quiet=False)
            os.system('unzip -qqq model_deduplication_extreme_dataset.zip  ')
        else:
            print('Dataset already downloadded')

def model(model_dir='models', task='all'):
    """Download pre-trained models.

    Args:
        model_dir: string, path to store download models
        task (str): all - download all models,
                    sentiment - download sentiment models, 
                    extremes - download extreme models
    """
    if not os.path.isdir(model_dir):
        os.makedirs(model_dir)
    # download sentiment classification models
    if task == 'all' or task =='sentiment':
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
        
        # w2v_wiki500_imdb_embed_trainable_up1
        if not os.path.isfile(os.path.join(model_dir, 'w2v_wiki500_imdb_embed_trainable_up1.h5')):
            model_url = 'https://drive.google.com/uc?id=1oyiGXyFdCvzRlRAM_yYkdzNcqnpTEHjx'
            gdown.download(model_url, output=None, quiet=False)
            os.system('mv w2v_wiki500_imdb_embed_trainable_up1.h5 ' + model_dir + '/')

        # w2v_wiki500_imdb_embed_trainable_up2
        if not os.path.isfile(os.path.join(model_dir, 'w2v_wiki500_imdb_embed_trainable_up2.h5')):
            model_url = 'https://drive.google.com/uc?id=1nXU5NkOwEXe-hCx6vh2HXgvFwllAK6WF'
            gdown.download(model_url, output=None, quiet=False)
            os.system('mv w2v_wiki500_imdb_embed_trainable_up2.h5 ' + model_dir + '/')

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

        # w2v_wiki250_civil_comment_embed_trainable
        if not os.path.isfile(os.path.join(model_dir, 'w2v_wiki250_civil_comment_embed_trainable.h5')):
            model_url = 'https://drive.google.com/uc?id=1Idxvo0Mt3ZSdJCr9XTaswgGT0IEGVU1o'
            gdown.download(model_url, output=None, quiet=False)
            os.system('mv w2v_wiki250_civil_comment_embed_trainable.h5 ' + model_dir + '/')

        # nnlm_en_dim128_yelp_embed_trainable.h5
        if not os.path.isfile(os.path.join(model_dir, 'nnlm_en_dim128_yelp_embed_trainable.h5')):
            model_url = 'https://drive.google.com/uc?id=1wm6qgWzqUOTTAoesHhv3aMRv1MartY2p'
            gdown.download(model_url, output=None, quiet=False)
            os.system('mv nnlm_en_dim128_yelp_embed_trainable.h5 ' + model_dir + '/')

        # nnlm_en_dim50_imdb_embed_trainable.h5
        if not os.path.isfile(os.path.join(model_dir, 'nnlm_en_dim50_imdb_embed_trainable.h5')):
            model_url = 'https://drive.google.com/uc?id=1-0-a0ldTD_BNr1cIZTU56AjJ0ypxjWw1'
            gdown.download(model_url, output=None, quiet=False)
            os.system('mv nnlm_en_dim50_imdb_embed_trainable.h5 ' + model_dir + '/')

    # download extreme classfication models
    if task == 'all' or task =='extreme':
        # eurlex_4.3k_xml
        if not os.path.isfile(os.path.join(model_dir, 'eurlex_4.3k_xml.h5')):
            model_url = 'https://drive.google.com/uc?id=1L6jOpxH81feR2R84CQybHOnysZiv1JXJ'
            gdown.download(model_url, output=None, quiet=False)
            os.system('mv eurlex_4.3k_xml.h5 ' + model_dir + '/')

        # rcv1_2k_xml
        if not os.path.isfile(os.path.join(model_dir, 'rcv1_2k_xml.h5')):
            model_url = 'https://drive.google.com/uc?id=1C6oqP20sRrGAQ3iAcT7Gd8kAarxA5eow'
            gdown.download(model_url, output=None, quiet=False)
            os.system('mv rcv1_2k_xml.h5 ' + model_dir + '/')

        # amazoncat_13k_xml
        if not os.path.isfile(os.path.join(model_dir, 'amazoncat_13k_xml.h5')):
            model_url = 'https://drive.google.com/uc?id=1nFQFJdTcMNK91k9kISG47az2OtVLKFvH'
            gdown.download(model_url, output=None, quiet=False)
            os.system('mv amazoncat_13k_xml.h5 ' + model_dir + '/')

        # amazoncat_14k_xml
        if not os.path.isfile(os.path.join(model_dir, 'amazoncat_14k_xml.h5')):
            model_url = 'https://drive.google.com/uc?id=1pMe-09R6doxSzCt4UMpgyd9pfBwX7jGm'
            gdown.download(model_url, output=None, quiet=False)
            os.system('mv amazoncat_14k_xml.h5 ' + model_dir + '/')
        

def main():
    download_dataset = True
    download_model = True
    download_task = 'all'
    if download_dataset:
        print('Downloading pre-processed datasets')
        dataset(task=download_task)
        print('Download dataset: Success!')
        
    if download_model:
        print('Downloading pre-trained models')
        model(task=download_task)
        print('Download pre-trained models: Success!')

if __name__ == "__main__":
    main()