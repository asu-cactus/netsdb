import argparse
import numpy as np
import torch
from torch import nn
from keras import layers, models
from utils import get_db_connection, create_tables, load_input_to_db, load_input_to_file

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
num_of_filters = kernel_dimensions[0]
stride = args.stride
load_data_from_file = True if args.loadFromFile == 'Y' else False
input_file_path = 'cnn-pytorch-input'
kernel_file_path = 'cnn-pytorch-kernel'

class Conv2DModel(torch.nn.Module):
    def __init__(self):
        super().__init__()
        self.network = nn.Sequential(
            nn.Conv2d(kernel_dimensions[1], kernel_dimensions[0], kernel_size = (kernel_dimensions[2], kernel_dimensions[3]), stride = stride),
        )

        kernels = torch.randn(*kernel_dimensions)
        self.network.weight = torch.nn.Parameter(kernels)

        print ("kernels", kernels.shape)
        print ("kernels", kernels.dtype)

    def forward(self, x):
        return self.network(x)

def saveModelPytorch():
    model = Conv2DModel()
    print (model)
    torch.save(model, 'conv2d-model.pth')

def saveModelTensorflow(kernel_dimensions, input_dimensions):
    # Create kernel weights
    kernels = np.random.rand(kernel_dimensions[2], kernel_dimensions[3], kernel_dimensions[1], kernel_dimensions[0]).astype('f')
    print ("filter", kernels.shape)
    print ("filter", kernels.dtype)
    
    # Create Model
    model = models.Sequential()
    model.add(layers.Conv2D(num_of_filters, (kernel_dimensions[2], kernel_dimensions[3]), input_shape=tuple(input_dimensions)))
    w = model.layers[0].get_weights()
    bias = w[1]
    model.layers[0].set_weights([kernels, bias])
    model.compile()
    model.save('conv2d-model')


if load_data_from_file:
    load_input_to_file(input_file_path, input_dimensions, number_of_images)
else:
    # connect to postgresql database
    db_connection = get_db_connection()
    db_cursor = db_connection.cursor()
    create_tables(db_cursor)

    # Create data into db  
    load_input_to_db(db_connection, input_dimensions, number_of_images)

# 2. Save model
saveModelPytorch()

saveModelTensorflow(kernel_dimensions, input_dimensions)

print ('Data saved')
