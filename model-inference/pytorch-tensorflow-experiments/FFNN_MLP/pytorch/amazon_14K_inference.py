import time
import torch
import torch.nn as nn
import numpy as np
import psycopg2

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

def get_db_connection():
    # connect to postgresql database
    t_host = "localhost"
    t_port = "5432"
    t_dbname = "postgres"
    t_user = "postgres"
    t_pw = "postgres"
    return psycopg2.connect(host=t_host, port=t_port, dbname=t_dbname, user=t_user, password=t_pw)

# Save input to DB using 
db_connection = get_db_connection()
db_cursor = db_connection.cursor()

# save the model
totalStart = time.time()
modelStart = time.time()
model = torch.load('Amazon14K.pth')
modelEnd = time.time()

# Load input from DB
inputStart = time.time()
db_cursor.execute(f"SELECT array_data FROM images WHERE id >= {0} and id < {batch}")
rows = db_cursor.fetchall()
data = []
for row in rows:
    row = torch.frombuffer(row[0], dtype=torch.double)
    row = row.reshape(input_dimension)
    data.append(row)

input = torch.stack(data)
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