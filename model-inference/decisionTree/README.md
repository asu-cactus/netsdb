# Benchmark Environment Setup

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

If you meet error "collect2: fatal error: cannot find ‘ld’", try to change the linker, e.g., you may change 'fuse-ld=lld' to 'fuse-ld=gold' in the ./CMakeFiles/tvm_runtime.dir/link.txt, ./CMakeFiles/tvm.dir/link.txt, and ./CMakeFiles/tvm_allvisible.dir/link.txt.

#### To Use PyTorch/TorchScript as backend

https://pytorch.org/

## PostgreSQL

https://www.postgresql.org/download/

## ConnectX

https://github.com/sfu-db/connector-x#installation
