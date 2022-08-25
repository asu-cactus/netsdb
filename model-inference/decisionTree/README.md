# Benchmark Environment Setup

## Python version
TVM requires Python version >= 3.6 and < 3.9. 

## Benchmark Platforms

### Scikit-Learn

https://scikit-learn.org/stable/install.html

### ONNX

https://onnxruntime.ai/docs/install/

### TreeLite

https://treelite.readthedocs.io/en/latest/install.html

### HummingBird

https://github.com/microsoft/hummingbird#installation

#### To Use TVM as backend

https://tvm.apache.org/docs/install/from_source.html


**There are two ways to build TVM -- build with CMake and buld with conda, see the documents in the above link for details**

If you build TVM with CMake, you may meet error "collect2: fatal error: cannot find ‘ld’", try to change the linker, e.g., you may change 'fuse-ld=lld' to 'fuse-ld=gold' in the ./CMakeFiles/tvm_runtime.dir/link.txt, ./CMakeFiles/tvm.dir/link.txt, and ./CMakeFiles/tvm_allvisible.dir/link.txt.

Remember to run 'make install' from the build directory after successfully compiling tvm to shared libraries.

#### To Use PyTorch/TorchScript as backend

https://pytorch.org/

## Tensorflow

https://www.tensorflow.org/install

## PostgreSQL

https://www.postgresql.org/download/

## ConnectX

https://github.com/sfu-db/connector-x#installation

## XGBoost

pip3 install xgboost

## LightGBM

pip3 install lightgbm

## Catboost

pip3 install catboost


