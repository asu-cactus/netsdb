# Prerequisite

## Install following packages (Ubuntu 20.04)
sudo apt update
sudo apt -y upgrade
sudo apt install python3-pip
pip3 install "tensorflow>=2.0.0"
pip3 install --upgrade tensorflow-hub
pip3 install tqdm

## Download the pre-trained word2vec embedding model

wget https://tfhub.dev/google/Wiki-words-500/2?tf-hub-format=compressed
mkdir saved_model
tar xvf 2\?tf-hub-format\=compressed -C saved_model/

## Run the scripts

(1) Configure following parameters:

    # Define the path to the text corpus to be trained
    path_to_file = tf.keras.utils.get_file('shakespeare.txt', 'https://storage.googleapis.com/download.tensorflow.org/data/shakespeare.txt')

    # Define the path to a vocabulary file, in which each word is placed in one line.
    # Such vocabulary file can be found as tokens.txt in the saved model of a pre-trained word embedding, under the folder called as assets
    path_to_vocab = "/home/ubuntu/saved_model/assets/tokens.txt"

    # Define the vocabulary size
    vocab_size = 1009375

    # Define the dimension of an embedding vector
    embedding_dim = 500

    # Define the number of words in a sequence
    sequence_length = 10

    # Define the number of negative samples for each target
    num_ns = 4

    # Define whether to use the text corpus to finetune the pretrained model or to train a new model from scratch
    finetune = True

(2) Run the script:

    python3  word2vec_main.py 
