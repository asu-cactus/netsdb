import psycopg2
import numpy as np
import pandas as pd
import csv

# create the data which will be saved into CSV and set it as double precision
A = np.random.rand(1000, 597540)
A = np.float64(A)

# save to CSV file
with open('input_double.csv', 'w', newline='') as file:
    mywriter = csv.writer(file, delimiter=',')
    mywriter.writerows(A)