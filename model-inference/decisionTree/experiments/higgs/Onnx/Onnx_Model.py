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

from skl2onnx import convert_sklearn
from skl2onnx.common.data_types import FloatTensorType
initial_type = [('float_input', FloatTensorType([None, 28]))]
onx = convert_sklearn(loaded_model, initial_types=initial_type)
with open("rf_higgs_100.onnx", "wb") as f:
    f.write(onx.SerializeToString())