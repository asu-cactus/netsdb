import time
import torch
import torch.nn as nn
import numpy as np
import connectorx as cx

input_dimension = 597540
batch = 1000

class Amazon14K(torch.nn.Module):
    def __init__(self):
            
            # calling constructor of parent class
            super().__init__()
            
            # defining the inputs to the first hidden layer
            self.hid1 = nn.Linear(597540, 1024, dtype=torch.double) 
            nn.init.normal_(self.hid1.weight, mean = 0, std = 2)
            nn.init.normal_(self.hid1.bias, mean = 0, std = 2)
            self.act1 = nn.ReLU()
            
            # defining the inputs to the third hidden layer
            self.hid2 = nn.Linear(1024, 14588, dtype=torch.double)
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
model = torch.load('Amazon14K.pth')
modelEnd = time.time()

# Load input from DB using connectorX
inputStart = time.time()
conn_string = "postgresql://postgres:postgres@localhost:5432/postgres"
query = f"SELECT * FROM images where id >= {0} and id < {1000}"
data = cx.read_sql(conn_string, query)
df = data["array_data"]

input = []
for bytes in df:
    currentBytes = np.frombuffer(bytes, dtype=np.double)
    currentBytes = currentBytes.reshape(input_dimension)
    input.append(torch.tensor(currentBytes, dtype=torch.double))
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