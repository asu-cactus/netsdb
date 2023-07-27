import connectorx as cx
import itertools
import numpy as np
import os
import pandas as pd
import time
import torch
from torch.utils.data import Dataset, DataLoader
from tqdm.auto import tqdm
from transformers import pipeline

class Timer(object):
    """A convenient class to measure the running time of a program

    """
    def __init__(self):
        self.start = 0
        self.end = 0
    
    def tic(self):
        """Tic the start time

        """
        self.start = time.perf_counter()
    
    def toc(self):
        """Toc the end time and return the running time

        Returns:
            float: running time (ms)
        """
        self.end = time.perf_counter()
        return (self.end - self.start)*1000

class my_dataset(Dataset):
    def __init__(self ,df):
        self.question = df['question'].tolist()
        self.context = df['context'].tolist()
    
    def __len__(self):
        return len(self.question)
    def __getitem__(self, idx):
        return {"question": self.question[idx],
                "context": self.context[idx]}

def load_squad_data_from_postgres(batch_size):
    conn_string = "postgresql://postgres:postgres@localhost:5432/postgres"
    query = "SELECT question, context FROM squad2 where index >= 0 and index < {}".format(batch_size)
    data = cx.read_sql(conn_string, query)
    return data

def load_squad_data_from_csv(batch_size):
    path_to_load = os.path.join('data', '{}.csv'.format(batch_size))
    data = pd.read_csv(path_to_load)
    return data


def load_data(data_source, batch_size):
    if data_source == 'postgres':
        data = load_squad_data_from_postgres(batch_size)
    elif data_source == 'csv':
        data = load_squad_data_from_csv(batch_size)
    
    return data

QA_input_warmup = {
    'question': 'Why is model conversion important?',
    'context': 'The option to convert models between FARM and transformers gives freedom to the user and let people easily switch between frameworks.'
}

num_run_loop = 100
num_warmup = 5
list_data_source = ['csv', 'postgres']
list_accelerater = [0, -1] # -1: use CPU, 0: use GPU:0
list_model_name = ["deepset/tinyroberta-squad2",
                   "deepset/roberta-base-squad2",
                   "deepset/bert-large-uncased-whole-word-masking-squad2"]
list_batch_size = [1, 10, 100, 1000, 10000, 100000]
list_run_config = list(itertools.product(list_data_source, list_accelerater, list_batch_size, list_model_name))

timer_load_data = Timer()
timer_inference = Timer()


# output an empty df to csv with header only, the results will be appended to this file
pd.DataFrame(columns=["data_source", "accelerater", "batch_size", "model_name", "t_load_data", "t_inference"]
             ).to_csv("latency_result.csv", mode='a', index=False, header=True)

for data_source, accelerater, batch_size, model_name in tqdm(list_run_config): 
    t_load_data = 0
    t_inference = 0
    try:
        for _ in range(num_run_loop):
            # load data
            timer_load_data.tic()
            
            data = load_data(data_source, batch_size)
            # note: for batching, if you want to run with torch Dataset, you need to
            # iterate over the result, otherwise the inference is not executed at all. 
            # another approach is useing dataloader as an example below.
            # results = []
            # for batch in data_loader:
            #    batch_outputs = nlp(batch)
            #    results.extend(batch)
            #
            my_dataset = my_dataset(data)
            data_loader = DataLoader(my_dataset, batch_size=batch_size, num_workers=4)
            
            t_load_data += timer_load_data.toc()

            # load model, we assume the model is cached in a real scenario
            # so we do not measure the time for loading the data
            nlp = pipeline('question-answering', model=model_name, 
                        tokenizer=model_name, device=accelerater)
            # perform an inference warmup 
            for _ in range(num_warmup):
                result = nlp(QA_input_warmup, batch_size=batch_size)
            
            # model inference
            timer_inference.tic() 
            for batch in data_loader:
                result = nlp(batch)
            t_inference += timer_inference.toc()
        
        # free gpu memory
        del nlp, result, my_dataset, data_loader
        torch.cuda.empty_cache()

        latency_df = pd.DataFrame({"data_source": [data_source], "accelerater": [accelerater], "batch_size": [batch_size], 
                                "model_name": [model_name], "t_load_data": [t_load_data/num_run_loop], 
                                "t_inference": [t_inference/num_run_loop]})
        latency_df.to_csv("latency_result.csv", mode='a', index=False, header=False);
    except Exception as e:
        # OOM exception
        latency_df = pd.DataFrame({"data_source": [data_source], "accelerater": [accelerater], "batch_size": [batch_size], 
                                "model_name": [model_name], "t_load_data": ["failed"], 
                                "t_inference": ["failed"]})
        latency_df.to_csv("latency_result.csv", mode='a', index=False, header=False);
