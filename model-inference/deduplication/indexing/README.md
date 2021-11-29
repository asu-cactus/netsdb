# Indexing for Duplication Detection

<!-- TOC -->

- [Indexing for Duplication Detection](#indexing-for-duplication-detection)
  - [Dependency](#dependency)
  - [Dataset](#dataset)
    - [Input and Labels](#input-and-labels)
    - [Download Preprocessed Dataset](#download-preprocessed-dataset)
  - [Models](#models)
    - [Train a model](#train-a-model)
  - [Automatically Download Datasets and Models](#automatically-download-datasets-and-models)
  - [How to Reproduce our Experiments](#how-to-reproduce-our-experiments)
    - [MISTIQUE Exact Match and Approximate Match](#mistique-exact-match-and-approximate-match)
    - [Enhanced Pairwise and Proposed Approach](#enhanced-pairwise-and-proposed-approach)

<!-- /TOC -->


## Dependency

```
conda create -n deduplication python=3.8.5
conda activate deduplication
pip install -r requirements.txt
```


## Dataset

We used three datasets, [imdb_reviews](https://www.tensorflow.org/datasets/catalog/imdb_reviews), [yelp_polarity_reviews](https://www.tensorflow.org/datasets/catalog/yelp_polarity_reviews), and [civil_comments](https://www.tensorflow.org/datasets/catalog/civil_comments), to train several text classification models.

### Input and Labels

yelp_polarity_reviews: the input is a review in text. The original dataset uses the label, `2`, to represent a positive review and the label, `1`, to represent a negative review. We preprocessed it as `1` to represent a positive review, while `0` represents a negative review.

imdb_reviews: the input is a review in text. The original datasets use the label, `1`, to represent a positive review and the label, `1`, to represent a negative review. We keep the same setting.

civil_comments: the input is a comment in text. The original datasets use a float point value, which ranges from 0 to 1, to indicate the toxicity of a comment. We set the comment's label whose toxicity is larger than `0.5` to `1`, otherwise, its label will be set as  `0`.


### Download Preprocessed Dataset

The preprocessed dataset can be downloaded via [here](https://drive.google.com/uc?id=1nYzDDSJGkjCsVQI4gbSdC3DafQ7Ez107).


## Models

We trained several models with different settings, specifically, the embedding layer is **trainable or not**.

The pre-trained model can also be downloaded as follow:

| Model                                     | Setting                                                                                     | Download Link                                                            |
| ----------------------------------------- | ------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------ |
| w2v_wiki500_yelp_embed_nontrainable       | Text classification model trained on yelp_review dataset with non-trainable embedding layer | [link](https://drive.google.com/uc?id=1-6T6c5MaaceARapMnPBEj0KP_P1-rg0y) |
| w2v_wiki500_yelp_embed_trainable          | Text classification model trained on yelp_review dataset with trainable embedding layer     | [link](https://drive.google.com/uc?id=1GgVaiexh643C7LlVH0qJHqlJxBZo71M0) |
| w2v_wiki500_imdb_embed_nontrainable       | Text classification model trained on imdb_review dataset with non-trainable embedding layer | [link](https://drive.google.com/uc?id=1jG2UjS75KG4pOeRKCWYdZd1o3TQlewfy) |
| w2v_wiki500_imdb_embed_trainable          | Text classification model trained on imdb_review dataset with trainable embedding layer     | [link](https://drive.google.com/uc?id=1-1I6r7kBQhwyHi5_Xatrwz_VFVM3H8GH) |
| w2v_wiki500_civil_comment_embed_trainable | Text classification model trained on civil_comment dataset with trainable embedding layer   | [link](https://drive.google.com/uc?id=1--bCnYYoe0mXseM783qqkncVMQVQmbhy) |

The embedding we used is: https://tfhub.dev/google/Wiki-words-500/2

### Train a model

The script we used to train a word classification model is wrapped up in [model_trainer.py](model_trainer.py).

You can train a model by setting `dataset` and `trainable` values. For example: `dataset=imdb_reviews` and `trainable=False` will output `w2v_wiki500_imdb_embed_nontrainable`.

Train a model with a specified setting by executing the following command 
```
python model_trainer.py
```

## Automatically Download Datasets and Models

You can change `download_dataset` and `download_model` settings in [downloader.py](downloader.py).
Execute the following code to automatically download the preprocessed datasets and pre-trained models.

```
python downloader.py
```

## How to Reproduce our Experiments

How we conduct the experiments is: we first create indexes of blocks from the first model. Then we perform the deduplication on the 2nd, 3rd, 4th, and 5th models.

| Model Name                                | Model Alias |
|-------------------------------------------|-------------|
| w2v_wiki500_yelp_embed_nontrainable       | Model 1     |
| w2v_wiki500_imdb_embed_nontrainable       | Model 2     |
| w2v_wiki500_imdb_embed_trainable          | Model 3     |
| w2v_wiki500_yelp_embed_trainable          | Model 4     |
| w2v_wiki500_civil_comment_embed_trainable | Model 5     |

Please refer to the sub-sections to see how to run the code on the models.

### MISTIQUE Exact Match and Approximate Match
The implementation of these two approaches is wrapped up in the `run_baseline.py` file. 

When running deduplication with two baselines, MISTIQUE Exact Match and MISTIQUE Approximate Match, you should set **method** to `exact_match` or `approximate_match` in [run_baseline.py](run_baseline.py).

Run the code:
```
python run_baseline.py
```

After running the code, the program will save one **csv** file and two **npy** files to the disk. The csv file is a tracking history of the block deduplication process. The npy of detector_output is an intermediate result that will be sent to page packing. *blocks_indexer.npy* is used to store the unique blocks.


To run it with new model, you should **replicate** function `baseline_run_on_second_model(method)` and modify it to the new model. You can refer to the example attached in the following section.

**Note**: after the first run, `baseline_run_on_first_model` can be commented out since the index should only be built once. Later, we will only invoke **update** operation.

### Enhanced Pairwise and Proposed Approach

The implementation of these two approaches are wrapped up in the `run.py` file. 

When running deduplication with different approaches, enhanced-pairwise, proposed w/o fine-tune or proposed w/ fine-tune, you should set **method** to `enhanced_pairwise`, `proposed_wo_finetune`, or `proposed_w_finetune` in [run.py](run.py).

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
python run.py
```
**Note**: after the first run, `run_on_first_model` can be commented out since the index should only be built once. Later, we will only invoke **update** operation.

After running the code, the program will save one **csv** file and two **npy** files to the disk. The csv file is a tracking history of the block deduplication process. The npy of detector_output is an intermediate result that will be sent to page packing. *blocks_indexer.npy* is used to store the unique blocks.