import time
import torch
import torch.nn as nn
import numpy as np
import connectorx as cx

input_dimension = 768
batch = 8000

class BERT(torch.nn.Module):
    def __init__(self):

            # calling constructor of parent class
            super().__init__()

            # defining the inputs to the first hidden layer
            self.hid1 = nn.Linear(768, 3072)
            nn.init.normal_(self.hid1.weight, mean = 0, std = 2)
            nn.init.normal_(self.hid1.bias, mean = 0, std = 2)
            self.act1 = nn.ReLU()

            # defining the inputs to the third hidden layer
            self.hid2 = nn.Linear(3072, 768)
            nn.init.normal_(self.hid2.weight, mean = 0, std = 2)
            nn.init.normal_(self.hid2.bias, mean = 0, std = 2)
            self.act2 = nn.Softmax()


    def forward(self, X):
            #input and act for layer 1
            X = self.hid1(X)
            X = self.act1(X)

            #input and act for layer 2
            X = self.hid2(X)
            X = self.act2(X)

            return X


# save the model
totalStart = time.time()
modelStart = time.time()
model = torch.load('keras_bert.pth')
modelEnd = time.time()

# Load input from DB using connectorX
inputStart = time.time()
conn_string = "postgresql://postgres:postgres@localhost:5432/postgres"
query = f"SELECT * FROM images where id >= {0} and id < {8000}"
data = cx.read_sql(conn_string, query)
df = data["array_data"]

input = []
for bytes in df:
    currentBytes = np.frombuffer(bytes, dtype=np.float)
    currentBytes = currentBytes.reshape(input_dimension)
    input.append(torch.tensor(currentBytes, dtype=torch.float))
input = torch.stack(input)
inputEnd = time.time()
totalEnd = time.time()

print ("Input shape", input.shape)
startTime = time.time()
output = model(input)
endTime = time.time()
print("Output Shape", output.shape)
totalEnd = time.time()

print ("Total Time:", totalEnd - totalStart)
print ("Sum Total:", (inputEnd - inputStart) + (modelEnd - modelStart) + (endTime - startTime))
print ("Input Load Time: ", inputEnd - inputStart)
print ("Model Load Time:", modelEnd - modelStart)
print ("Inference Time:", endTime - startTime)
