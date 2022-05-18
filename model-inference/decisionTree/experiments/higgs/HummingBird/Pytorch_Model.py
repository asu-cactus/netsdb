# transfer to onnx model and save
import pandas
import pickle
import joblib
import torch
import torchvision
from hummingbird.ml import convert
import numpy as np
import time
import onnx
import onnxruntime as ort
from onnxmltools import convert_sklearn
from onnxconverter_common import FloatTensorType

filename = 'rf-100-8-6.pkl'
loaded_model = joblib.load(filename)
model = convert(loaded_model,'pytorch')
torch.save(model, 'rf-100-8-6_torch.pkl')