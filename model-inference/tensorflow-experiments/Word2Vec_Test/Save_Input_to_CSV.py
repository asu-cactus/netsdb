import psycopg2
import numpy as np
import pandas as pd
import csv

vocab_size = 1009375

# create the data which will be saved into CSV and set it as double precision
# B is used for matrix multiplication
B = np.random.rand(100, vocab_size)
B = np.float64(B)

# save to CSV file
with open('input_double.csv', 'w', newline='') as file:
    mywriter = csv.writer(file, delimiter=',')
    mywriter.writerow(B)

# create the data which will be saved into CSV and set it as float32 precision
# A is used for look up function
A = np.random.randint(0, vocab_size-1, (1000000,))
A = np.float32(A)

# save to CSV file
with open('input_float.csv', 'w', newline='') as file:
    mywriter = csv.writer(file, delimiter=',')
    mywriter.writerow(A)