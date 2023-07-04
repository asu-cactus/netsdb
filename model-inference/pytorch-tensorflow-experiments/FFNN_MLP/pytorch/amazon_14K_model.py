import time
import torch
import torch.nn as nn
import numpy as np
import psycopg2

input_dimension = 597540
input_path = 'amazon_14K_Input'
batch = 1000

class Amazon14K(torch.nn.Module):
    def __init__(self):
            
            # calling constructor of parent class
            super().__init__()
            
            # defining the inputs to the first hidden layer
            self.hid1 = nn.Linear(input_dimension, 1024, dtype=torch.double) 
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

# Save model
model = Amazon14K()
print (model)
torch.save(model, 'Amazon14K.pth')

# Save input to DB
def get_db_connection():
    # connect to postgresql database
    t_host = "localhost"
    t_port = "5432"
    t_dbname = "postgres"
    t_user = "postgres"
    t_pw = "postgres"
    return psycopg2.connect(host=t_host, port=t_port, dbname=t_dbname, user=t_user, password=t_pw)

db_connection = get_db_connection()
db_cursor = db_connection.cursor()

db_cursor.execute("DROP TABLE IF EXISTS images;")
db_cursor.execute("CREATE TABLE IF NOT EXISTS images(id int, array_data bytea);")

for id in range(0, batch):
    img = np.random.rand(input_dimension)
    img_bytes = img.tobytes()
    db_cursor.execute("INSERT INTO images(id,array_data) " + "VALUES(%s,%s)",(id, img_bytes))

# commit the changes to the database
db_connection.commit()
db_cursor.close()
