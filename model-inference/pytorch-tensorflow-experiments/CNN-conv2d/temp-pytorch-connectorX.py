import time
import argparse
import connectorx as cx
import numpy as np
import torch
from torch import nn
import torch.nn.functional as F
from utils import get_db_connection

# defaults
default_number_of_images = 100
# width, height,channels
default_input_size = "224, 224, 3"
# number of filters, channels, width, height
default_kernel_size = "64, 3, 7, 7"
# stride along width and height
default_stride = 1
# load input data from file
default_load_data = 'N'
default_repetitions = 1
# load number of images
default_batch_size = 1

parser = argparse.ArgumentParser()
parser.add_argument("--images", type=int, help="total number of input images", default=default_number_of_images)
parser.add_argument("--inputsize", type=str, help=f"input size 'batchsize, channels, width, height', default value = '{default_input_size}'", default=default_input_size)
parser.add_argument("--kernelsize", type=str, help=f"kernel size 'number of kernels, channels, width, height', default value = '{default_kernel_size}'", default=default_kernel_size)
parser.add_argument("--stride", type=int, help=f"stride along width, height', default value = '{default_stride}'", default=default_stride)
parser.add_argument("--loadFromFile", type=str, help=f"load data from file Y: Yes, N: No', default value = {default_load_data}'", default=default_load_data)
parser.add_argument("--repetitions", type=int, help=f"repeat experiment many time and report average time= {default_repetitions}'", default=default_repetitions)
parser.add_argument("--batch", type=int, help=f"read number of images at once= {default_batch_size}'", default=default_batch_size)
parser.add_argument('--connectorX', action=argparse.BooleanOptionalAction)
args = parser.parse_args()

# constants
number_of_images = args.images
kernel_id = 0
repetitions = args.repetitions
batch_size = args.batch
useConnectorX = args.connectorX

input_dimensions = [int(item) for item in args.inputsize.split(',')]
kernel_dimensions = [int(item) for item in args.kernelsize.split(',')]
stride = args.stride
load_data_from_file = True if args.loadFromFile == 'Y' else False
input_file_path = 'cnn-pytorch-input'
kernel_file_path = 'cnn-pytorch-kernel'


def printTime(kernelLoadTime, inputLoadTime, conv2dOpTime):
    total_time = kernelLoadTime + inputLoadTime + conv2dOpTime
    print("Total time duration: ", total_time)
    print("Kernel load duration: ", kernelLoadTime)
    print("Input load duration: ", inputLoadTime)
    print("Conv2d ops duration: ", conv2dOpTime)

# connection string
conn_string = "postgresql://postgres:postgres@localhost:5432/postgres"

# 1. load model
startModelLoad = time.time()
model = torch.load('conv2d-model.pth')
endModelLoad = time.time()
modelLoadTime = endModelLoad - startModelLoad

# 2. Read input data from db
inputLoadTime = 0
conv2dOpTime = 0

for id in range(0, number_of_images, batch_size):
    inputStartTime = time.time()
    input = []
    currentId = id

    query = f"SELECT * FROM images where id >= {currentId} and id < {currentId + batch_size}"
    data = cx.read_sql(conn_string, query)

    df = data["array_data"]
    
    for bytes in df:
        currentBytes = np.frombuffer(bytes, dtype=np.float32)
        currentBytes = currentBytes.reshape(input_dimensions[2], input_dimensions[0], input_dimensions[1])
        input.append(torch.tensor(currentBytes, dtype=torch.float32))

    input = torch.stack(input)
    inputEndTime = time.time()
    inputLoadTime = inputLoadTime + (inputEndTime - inputStartTime)

    # Log Input
    print ("input", input.shape)
    print ("input", input.dtype)

    # 5. Perform Conv2d operation
    startTime = time.time()
    output = model(input)
    endTime = time.time()
    conv2dOpTime = conv2dOpTime + (endTime - startTime)
    
    # Log Output
    print ("Output Shape: ", output.shape)

# Print results
printTime(modelLoadTime, inputLoadTime, conv2dOpTime)