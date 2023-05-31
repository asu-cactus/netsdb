import psycopg2
import torch
import time
import argparse
from utils import get_db_connection, create_tables, load_input_to_db, load_input_to_file_torch, load_kernel_to_file_torch, read_input_from_db, read_input_from_db_cx

# defaults
default_number_of_images = 100
# batchsize, channels, width, height
default_input_size = "10, 3, 224, 224"
# out_channels, in_channels, height, width
default_kernel_size = "64, 3, 7, 7"
# stride along width and height
default_stride = 1
# load input data from file
default_load_data = 'N'
# db option
default_db_option = 'cx'

parser = argparse.ArgumentParser()
parser.add_argument("--images", type=int, help="total number of input images", default=default_number_of_images)
parser.add_argument("--inputsize", type=str, help=f"input size 'batchsize, channels, width, height', default value = '{default_input_size}'", default=default_input_size)
parser.add_argument("--kernelsize", type=str, help=f"kernel size 'number of kernels, channels, height, width', default value = '{default_kernel_size}'", default=default_kernel_size)
parser.add_argument("--stride", type=int, help=f"stride along width, height', default value = '{default_stride}'", default=default_stride)
parser.add_argument("--loadFromFile", type=str, help=f"load data from file Y: Yes, N: No', default value = {default_load_data}'", default=default_load_data)
parser.add_argument("--db", type=str, help=f"database for storing the feature vectors 'postgres or cx', default value = '{default_db_option}'", default=default_db_option)
args = parser.parse_args()

# constants
number_of_images = args.images
kernel_id = 0

input_dimensions = [int(item) for item in args.inputsize.split(',')]
kernel_dimensions = [int(item) for item in args.kernelsize.split(',')]
stride = args.stride
load_data_from_file = True if args.loadFromFile == 'Y' else False
input_file_path = 'cnn-pytorch-input'
kernel_file_path = 'cnn-pytorch-kernel'
db = args.db

_iterations = number_of_images
# connect to postgresql database
db_connection = get_db_connection()
db_cursor = db_connection.cursor()

# create the table named images and kernel
create_tables(db_cursor)

# load the input and kernel to PostgreSQL DB/File
try:
    if load_data_from_file:
        load_input_to_file_torch(input_file_path, input_dimensions, _iterations)
    else:
        load_input_to_db(db_connection, input_dimensions, _iterations)

    load_kernel_to_file_torch(kernel_file_path, kernel_dimensions)
except(Exception, psycopg2.DatabaseError) as error:    
    print("error while loading data", error)

try:
    # read kernel data
    startKernelLoad = time.time()
    filter = torch.load(kernel_file_path + '.pt')
    endKernelLoad = time.time()
    kernelLoadTime = endKernelLoad - startKernelLoad
    
    # TODO: Add bias since conv2d includes bias
    # bias = torch.randn(kernel_dimensions[0], dtype=torch.float32)

    # read input data
    inputLoadTime = 0
    conv2dOpTime = 0
    for id in range(0, _iterations):
        startTime = time.time()
        if load_data_from_file:
            input = torch.load(input_file_path + str(id) + '.pt')
        elif db == 'postgres':
            input = read_input_from_db(db_cursor, id, input_dimensions)
            input = torch.tensor(input, dtype=torch.float32) # TODO: find optimal implementation for this. 
        elif db == 'cx':
            input = read_input_from_db_cx(db_cursor, id, input_dimensions)
            input = torch.tensor(input, dtype=torch.float32) # TODO: find optimal implementation for this.
        endTime = time.time()
        inputLoadTime = inputLoadTime + (endTime - startTime)

        print ("input", input.shape)
        print ("input", input.dtype)
        print ("filter", filter.shape)
        print ("filter", filter.dtype)
        print ("bias", bias.shape)
        print ("bias", bias.dtype)

        startTime = time.time()
        output = torch.nn.functional.conv2d(input, filter, stride=stride)
        endTime = time.time()
        print(output)
        conv2dOpTime = conv2dOpTime + (endTime - startTime)
        print ("Output Shape: ", output.shape)
except(Exception, psycopg2.DatabaseError) as error:
    print ("exception while reading images", error)
finally:
    if db_connection is not None:
        db_connection.close()

# close the communication with the PostgresQL database
db_cursor.close()

print("Total time duration: ", kernelLoadTime + inputLoadTime + conv2dOpTime)
print("Kernel load duration: ", kernelLoadTime)
print("Input load duration: ", inputLoadTime)
print("Conv2d ops duration: ", conv2dOpTime)
