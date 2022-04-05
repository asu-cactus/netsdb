# Indexing for Duplication Detection

<!-- TOC -->

- [Indexing for Duplication Detection](#indexing-for-duplication-detection)
  - [Dependency](#dependency)
  - [Task](#task)
    - [Sentiment Text Classification](#sentiment-text-classification)
      - [Dataset Introduction & Pre-Processing](#dataset-introduction--pre-processing)
      - [Access Preprocessed Dataset](#access-preprocessed-dataset)
      - [Model Training](#model-training)
    - [Train a model](#train-a-model)
    - [Extreme Text (Multi-Label) Classification](#extreme-text-multi-label-classification)
      - [Dataset Introduction & Pre-Processing](#dataset-introduction--pre-processing-1)
      - [Access Dataset](#access-dataset)
  - [Automatically Download Datasets and Models](#automatically-download-datasets-and-models)
  - [How to Reproduce our Experiments](#how-to-reproduce-our-experiments)
    - [MISTIQUE Exact Match and Approximate Match](#mistique-exact-match-and-approximate-match)
    - [Enhanced Pairwise and Proposed Approach](#enhanced-pairwise-and-proposed-approach)

<!-- /TOC -->


## Dependency

```
conda create -n deduplication python=3.8
conda activate deduplication
pip install -r requirements.txt
```


## Task

### Sentiment Text Classification 

#### Dataset Introduction & Pre-Processing

We trained several models on the following three datasets ([imdb_reviews](https://www.tensorflow.org/datasets/catalog/imdb_reviews), [yelp_polarity_reviews](https://www.tensorflow.org/datasets/catalog/yelp_polarity_reviews), and [civil_comments](https://www.tensorflow.org/datasets/catalog/civil_comments)) to do sentiment classification. 

|  **Dataset**  |   **Input**   |           **Label**           | **Number of Training Point** | **Number of Testing Point** |
|:-------------:|:-------------:|:-----------------------------:|:----------------------------:|:---------------------------:|
|  IMDB Review  | text sentenct | 0 for negative 1 for positive |            40,000            |            10,000           |
|  Yelp Review  | text sentenct | 0 for negative 1 for positive |            478,400           |           119,600           |
| Civil Comment | text sentenct | 0 for negative 1 for positive |           1,521,755          |           380,439           |

The reviews of these three comments are all stored as sentenct. We applied data pre-processing to generate the sentiment label for each sentenc, which is defined as below:

- yelp_polarity_reviews: The original dataset uses the label, `2`, to represent a positive review and the label, `1`, to represent a negative review. We preprocessed it as `1` to represent a positive review, while `0` represents a negative review.

- imdb_reviews: The original datasets use the label, `1`, to represent a positive review and the label, `1`, to represent a negative review. We keep the same setting.

- civil_comments: The original datasets use a float point value, which ranges from 0 to 1, to indicate the toxicity of a comment. We set the comment's label whose toxicity is larger than `0.5` to `1`, otherwise, its label will be set as  `0`.

#### Access Preprocessed Dataset

The preprocessed dataset can be downloaded via [here](https://drive.google.com/uc?id=1nYzDDSJGkjCsVQI4gbSdC3DafQ7Ez107). Or, you can use [downloader](downloader.py) to download the preprocessed dataset.

#### Model Training

The architecture we used to train a model is: an Embedding Layer (trainable/not-trainable weights) + 2 Fully Connected Layers. You can train the model from stractch by utilizing our [model_trainer_sentiment_task](model_trainer_sentiment_task.py). Or you can use [downloader](downloader.py) to download our pre-trained models.

The sentiment classification models that we trained are summarized in the following table.


| Model                                     | Embedding Layer                                              | Setting                                                                                     | Download Link                                                            |
|-------------------------------------------|--------------------------------------------------------------|---------------------------------------------------------------------------------------------|--------------------------------------------------------------------------|
| w2v_wiki500_yelp_embed_nontrainable       | [w2v_wiki500](https://tfhub.dev/google/Wiki-words-500/2)     | Text classification model trained on yelp_review dataset with non-trainable embedding layer | [link](https://drive.google.com/uc?id=1-6T6c5MaaceARapMnPBEj0KP_P1-rg0y) |
| w2v_wiki500_yelp_embed_trainable          | [w2v_wiki500](https://tfhub.dev/google/Wiki-words-500/2)     | Text classification model trained on yelp_review dataset with trainable embedding layer     | [link](https://drive.google.com/uc?id=1GgVaiexh643C7LlVH0qJHqlJxBZo71M0) |
| w2v_wiki500_imdb_embed_nontrainable       | [w2v_wiki500](https://tfhub.dev/google/Wiki-words-500/2)     | Text classification model trained on imdb_review dataset with non-trainable embedding layer | [link](https://drive.google.com/uc?id=1jG2UjS75KG4pOeRKCWYdZd1o3TQlewfy) |
| w2v_wiki500_imdb_embed_trainable          | [w2v_wiki500](https://tfhub.dev/google/Wiki-words-500/2)     | Text classification model trained on imdb_review dataset with trainable embedding layer     | [link](https://drive.google.com/uc?id=1-1I6r7kBQhwyHi5_Xatrwz_VFVM3H8GH) |
| w2v_wiki500_civil_comment_embed_trainable | [w2v_wiki500](https://tfhub.dev/google/Wiki-words-500/2)     | Text classification model trained on civil_comment dataset with trainable embedding layer   | [link](https://drive.google.com/uc?id=1--bCnYYoe0mXseM783qqkncVMQVQmbhy) |
| nnlm_en_dim_128_yelp_embed_trainable      | [nnlm_en_dim_128](https://tfhub.dev/google/nnlm-en-dim128/2) | Text classification model trained on yelp_review dataset with trainable embedding layer     | [link](https://drive.google.com/uc?id=1wm6qgWzqUOTTAoesHhv3aMRv1MartY2p) |
| nnlm_en_dim_50_imdb_embed_trainable       | [nnlm_en_dim_50](https://tfhub.dev/google/nnlm-en-dim50/2)   | Text classification model trained on imdb_review dataset with trainable embedding layer     | [link](https://drive.google.com/uc?id=1-0-a0ldTD_BNr1cIZTU56AjJ0ypxjWw1) |
| w2v_wiki250_civil_comment_embd_trainable  | [w2v_wiki250](https://tfhub.dev/google/Wiki-words-250/2)     | Text classification model trained on civil_comment dataset with trainable embedding layer   | [link](https://drive.google.com/uc?id=1Idxvo0Mt3ZSdJCr9XTaswgGT0IEGVU1o) |
| w2v_wiki500_imdb_embed_trainable_up1      | [w2v_wiki500](https://tfhub.dev/google/Wiki-words-500/2)     | Train one more epoch on the training data                                                   | [link](https://drive.google.com/uc?id=1oyiGXyFdCvzRlRAM_yYkdzNcqnpTEHjx) |
| w2v_wiki500_imdb_embed_trainable_up2      | [w2v_wiki500](https://tfhub.dev/google/Wiki-words-500/2)     | Train two more epoch on the training data                                                   | [link](https://drive.google.com/uc?id=1nXU5NkOwEXe-hCx6vh2HXgvFwllAK6WF) |

The script we used to train a sentiment classification model is wrapped up in [model_trainer_sentiment_task.py](model_trainer_sentiment_task.py).

You can change the training parameter in the `main` function to train model for specifc python then execute the following command to start model training.

```
python model_trainer_sentiment_task.py
```


### Extreme Text (Multi-Label) Classification 
  
|  **Dataset**  | **Bag of Words Feature Dimensionality** | **Number of Labels** | **Number of Training Point** | **Number of Testing Point** |
|:-------------:|:---------------------------------------:|:--------------------:|:----------------------------:|:---------------------------:|
|    RCV1-2K    | 47,236                                  | 2,456                | 623,847                      | 155,962                     |
|  EURLex-4.3K  | 200,000                                 | 4,271                | 45,000                       | 6,000                       |
| AmazonCat-13K | 203,882                                 | 13,330               | 1,186,239                    | 306,782                     |
| AmazonCat-14K | 597,540                                 | 14,588               | 4,398,050                    | 1,099,725                   |

#### Dataset Introduction & Pre-Processing

The dataset we used is hosted on [Extreme Classification Repository](http://manikvarma.org/downloads/XC/XMLRepository.html), please check the details of the datasets we used there.

#### Access Dataset

We did not pre-process the dataset. Hence you can download the dataset from [Extreme Classification Repository](http://manikvarma.org/downloads/XC/XMLRepository.html) or use [downloader](downloader.py) to download the dataset.

#### Model Training 

The architecture we used to train a model is: an input layer + a hidden layer + an output layer. You can train the model from stractch by utilizing our [model_trainer_extreme_task](model_trainer_extreme_task.py). Or you can use [downloader](downloader.py) to download our pre-trained models.

| Model             | Description                                                               | Download Link                                                            |
|-------------------|---------------------------------------------------------------------------|--------------------------------------------------------------------------|
| eurlex_4.3k_xml   | a 2-layer fully connected neural network trained on eurlex_4.3k dataset   | [link](https://drive.google.com/uc?id=1L6jOpxH81feR2R84CQybHOnysZiv1JXJ) |
| rcv1x_2k_xml      | a 2-layer fully connected neural network trained on rcv1x_2k dataset      | [link](https://drive.google.com/uc?id=1C6oqP20sRrGAQ3iAcT7Gd8kAarxA5eow) |
| amazoncat_13k_xml | a 2-layer fully connected neural network trained on amazoncat-13k dataset | [link](https://drive.google.com/uc?id=1nFQFJdTcMNK91k9kISG47az2OtVLKFvH) |
| amazoncat_14k_xml | a 2-layer fully connected neural network trained on amazoncat-14k dataset | [link](https://drive.google.com/uc?id=1pMe-09R6doxSzCt4UMpgyd9pfBwX7jGm) |


The script we used to train a sentiment classification model is wrapped up in [model_trainer_extreme_task.py](model_trainer_extreme_task.py).

You can change the training parameter in the `main` function to train model for specifc python then execute the following command to start model training.

```
python model_trainer_extreme_task.py
```


## Utils to Download Datasets and Models

Please change the parameters in the `main` function of [downloader.py](downloader.py) to download the dataset/model you need. Then executeing the following command:

```
python downloader.py
```

## Run Deduplication

How we conduct the experiments is: we first create indexes of blocks from the first model. Then we perform the deduplication on the 2nd, 3rd and following models. 
For example, we will deduplicate the following five different sentiment classification models.

| Model Name                                | Model Alias |
|-------------------------------------------|-------------|
| w2v_wiki500_yelp_embed_nontrainable       | Model 1     |
| w2v_wiki500_imdb_embed_nontrainable       | Model 2     |
| w2v_wiki500_imdb_embed_trainable          | Model 3     |
| w2v_wiki500_yelp_embed_trainable          | Model 4     |
| w2v_wiki500_civil_comment_embed_trainable | Model 5     |

### MISTIQUE Exact Match and Approximate Match
The implementation of these two approaches is wrapped up in the `run_baseline.py` file. 

When running deduplication with two baselines, MISTIQUE Exact Match and MISTIQUE Approximate Match, you should set **method** to `exact_match` or `approximate_match` in [run_baseline.py](run_baseline.py).

Run the code:
```
python run_baseline.py
```

After running the code, the program will save one **csv** file and two **npy** files to the disk. The csv file is a tracking history of the block deduplication process. The npy of detector_output is an intermediate result that will be sent to page packing. *blocks_indexer.npy* is used to store the unique blocks.


To run it with new model, you should **replicate** function `baseline_run_on_second_model(method)` and modify it to the new model. You can refer to the example attached in the following section.

**Note**: after the first run, `baseline_run_on_first_model` can be commented out. Later, we will invoke **Indexer.update()** operation to update the index.

### Enhanced Pairwise and Proposed Approach

The implementation of these two approaches are wrapped up in the `run_dedup_w2v_classifier.py` file. 

When running deduplication with different approaches, enhanced-pairwise, proposed w/o fine-tune or proposed w/ fine-tune, you should set **method** to `enhanced_pairwise`, `proposed_wo_finetune`, or `proposed_w_finetune` in [run_dedup_w2v_classifier.py](run_dedup_w2v_classifier.py).

To run it with new model, you should **replicate** function `run_on_second_model(method)` and modify it to the new model. For example, I want to run it with the Model 3. I should add the following code:

```python
def run_on_third_model(method):
    model_path = 'models'

    # define block size
    block_size_x = 10000
    block_size_y = 100

    # create indexer
    blocks_indexer = indexer.Indexer(block_size_x=block_size_x, block_size_y=block_size_y)
    blocks_indexer.load('blocks_indexer.npy')

    # run deduplication on w2v_wiki500_imdb_embed_trainable
    # load model
    m2 = tf.keras.models.load_model(os.path.join(model_path, 'w2v_wiki500_imdb_embed_trainable.h5'), 
                                    custom_objects={'KerasLayer':hub.KerasLayer})
    m2_ms = blocker.block_model_2d(m2, block_size_x=block_size_x, block_size_y=block_size_y)

    # load dataset for imdb
    imdb_x, imdb_y = model_trainer.load_dataset(dataset="imdb_reviews")
    imdb_x_train, imdb_x_test, imdb_y_train, imdb_y_test = train_test_split(
                                        imdb_x, imdb_y, test_size=0.2, random_state=0)

    if method == 'enhanced_pairwise':
        # run deduplication (enhanced-pairwise)
        result_df = deduplicator.deduplicate_model(m2, m2_ms, imdb_x_test, imdb_y_test, 
                                    blocks_indexer, fp=0.01, sim=0.7, stop_acc_drop=0.035, 
                                    use_lsh=False)
    elif method == 'proposed_wo_finetune':
        # run deduplication (proposed w/o fine-tune)
        result_df = deduplicator.deduplicate_model(m2, m2_ms, imdb_x_test, imdb_y_test, 
                                    blocks_indexer, fp=0.01, sim=0.7, stop_acc_drop=0.035, 
                                    use_lsh=True)
    elif method == 'proposed_w_finetune':
        # run deduplication (proposed w/ fine-tune)
        result_df = deduplicator.deduplicate_model(m2, m2_ms, imdb_x_test, imdb_y_test, 
                                    blocks_indexer, fp=0.01, sim=0.7, stop_acc_drop=0.035, 
                                    use_lsh=True, x_train=imdb_x_train, y_train=imdb_y_train)

    detector_output, num_unique_blocks = deduplicator.generate_detector_output(result_df, m2_ms, blocks_indexer.num_total)
    result_df.to_csv('imdb_embed_trainable_result.csv', index=False)
    np.save('imdb_embed_trainable_output.npy', detector_output)

    # update index
    blocks_indexer.update_index(m2_ms, result_df)

    # save to disk
    blocks_indexer.save('blocks_indexer.npy')

```

Run the code:
```
python run_dedup_w2v_classifier.py
```
**Note**: after the first run, `run_on_first_model` can be commented out. Later, we will invoke **Indexer.update()** operation to update the index.

After running the code, the program will save one **csv** file and two **npy** files to the disk. The csv file is a tracking history of the block deduplication process. The npy of detector_output is an intermediate result that will be sent to page packing. *blocks_indexer.npy* is used to store the unique blocks.

### Deduplicate sentiment classification models with different embedding layer

We perform the deduplication on the following hetergeneous sentiment classification models. The code is wrapped in [run_hetero_model_exp1](run_hetero_model_exp1).

|                   Model                  | Embedding Layer Size |
|:----------------------------------------:|:--------------------:|
|   nnlm_en_dim_128_yelp_embed_trainable   |      963812, 128     |
|    nnlm_en_dim_50_imdb_embed_trainable   |      963812, 50      |
| w2v_wiki250_civil_comment_embd_trainable |     1009375, 250     |
|     w2v_wiki500_yelp_embed_trainable     |     1009375, 500     |

### Deduplicate different extreme classification models

We perform the deduplication on the following hetergeneous extreme classification models. The code is wrapped in [run_hetero_model_exp2](run_hetero_model_exp2).

|       Model       | Download Link (input layer, hidden layer, output layer) |
|:-----------------:|:-------------------------------------------------------:|
|  eurlex_4.3k_xml  |                    200000, 2000, 4271                   |
|    rcv1x_2k_xml   |                    47236, 5000, 2456                    |
| amazoncat_13k_xml |                    203882, 1000, 1330                   |
| amazoncat_14k_xml |                   597540, 1000, 14588                   |

### Deduplicate pairs of sentiment classification model and extreme classification model

#### nnlm128_yelp + rcv1x_2k_xml

The code is wrapped in [run_hetero_model_exp3](run_hetero_model_exp3).

#### wiki500_yelp + eurlex_4.3k_xml

The code is wrapped in [run_hetero_model_exp4](run_hetero_model_exp4).

#### wiki500_yelp + amazoncat_13k_xml

The code is wrapped in [run_hetero_model_exp5](run_hetero_model_exp5).

### Deduplicate updated models

#### w2v_wiki500_imdb_embed_trainable_update1

Deduplicate the model that is trained 1 more epoch on the imdb dataset. We compare the deduplication result of two different strategies: (1) deduplicate the model as a new model (2) only deduplicate the blocks whose has at least N LSH signatures get changed.

The code is wrapped up in [run_update.py](run_update.py).

#### w2v_wiki500_imdb_embed_trainable_update2

Deduplicate the model that is trained 2 more epochs on the imdb dataset. We compare the deduplication result of two different strategies: (1) deduplicate the model as a new model (2) only deduplicate the blocks whose has at least N LSH signatures get changed.

The code is wrapped up in [run_update.py](run_update.py).

### Deduplication with normalization

We compare the deduplication with different normalization strategies:

1. Normalize by layer, deduplicate by layer
2. Normalize by layer, deduplicate together
3. No normalization, deduplicate by layer
4. No normalization, deduplicate together

The code is wrapped up in [run_cross_layer_dedup.py](run_cross_layer_dedup.py).