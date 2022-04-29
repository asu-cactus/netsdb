import time
import torch
import torch.nn as nn

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

model = BERT()
input_dimension = 768
batch = 1000

input = torch.rand(batch, input_dimension)
startTime = time.time()
output = model(input)
endTime = time.time()
print("Output Shape", output.shape)
print ("Time:", endTime - startTime)