import psycopg2
import numpy as np
import tensorflow as tf
import time
import argparse
from utils import get_db_connection, create_tables, load_input_to_db, load_input_to_file, load_kernel_to_file, read_kernel_data, read_input_from_db

# defaults
default_number_of_images = 100
# batchsize, channels, width, height
default_input_size = "10, 224, 224, 3"
# number of filters, channels, width, height
default_kernel_size = "7, 7, 3, 64"
# stride along width and height
default_stride = 1
# load input data from file
default_load_data = 'N'

parser = argparse.ArgumentParser()
parser.add_argument("--images", type=int, help="total number of input images", default=default_number_of_images)
parser.add_argument("--inputsize", type=str, help=f"input size 'batchsize, channels, width, height', default value = '{default_input_size}'", default=default_input_size)
parser.add_argument("--kernelsize", type=str, help=f"kernel size 'number of kernels, channels, width, height', default value = '{default_kernel_size}'", default=default_kernel_size)
parser.add_argument("--stride", type=int, help=f"stride along width, height', default value = '{default_stride}'", default=default_stride)
parser.add_argument("--loadFromFile", type=str, help=f"load data from file Y: Yes, N: No', default value = {default_load_data}'", default=default_load_data)
args = parser.parse_args()

# constants
number_of_images = args.images
kernel_id = 0

input_dimensions = [int(item) for item in args.inputsize.split(',')]
kernel_dimensions = [int(item) for item in args.kernelsize.split(',')]
stride = args.stride
load_data_from_file = True if args.loadFromFile == 'Y' else False
input_file_path = 'cnn-tensorflow-input'
kernel_file_path = 'cnn-tensorflow-kernel'

_iterations = number_of_images
# connect to postgresql database
db_connection = get_db_connection()
db_cursor = db_connection.cursor()

# create the table named images and kernel
create_tables(db_cursor)

# load the input and kernel to PostgreSQL DB/File
try:
    if load_data_from_file:
        load_input_to_file(input_file_path, input_dimensions, _iterations)
    else:
        load_input_to_db(db_connection, input_dimensions, _iterations)
        # load_kernel_to_db(db_connection, kernel_dimensions, kernel_id)

    load_kernel_to_file(kernel_file_path, kernel_dimensions)
except(Exception, psycopg2.DatabaseError) as error:    
    print("error while loading data", error)


# print ("before try block")
try:
    # read kernel data
    startKernelLoad = time.time()
    filter = read_kernel_data(True, kernel_file_path, None, kernel_id, kernel_dimensions)
    endKernelLoad = time.time()
    kernelLoadTime = endKernelLoad - startKernelLoad
    
    inputLoadTime = 0
    conv2dOpTime = 0
    # read input data
    for id in range(0, _iterations):
        startTime = time.time()
        if load_data_from_file:
            input = np.load(input_file_path + str(id) + '.npy')
        else:
            input = read_input_from_db(db_cursor, id, input_dimensions)
        endTime = time.time()
        inputLoadTime = inputLoadTime + (endTime - startTime)

        print ("input", input.shape)
        print ("filter", filter.shape)

        startTime = time.time()
        output = tf.nn.conv2d(input, filter, strides=stride, padding='VALID')
        endTime = time.time()
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
