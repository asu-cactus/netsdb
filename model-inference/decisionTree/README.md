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

### LLeaves (Model Compiler for LightGBM Model)

pip3 install lleaves (or) conda install -c conda-forge lleaves

## Catboost (This is not a framework. Only used to load the "epsilon" dataset)

pip3 install catboost

# Generating Synthetic Data using TPCxAI
* Tool Download Link: [TPCxAI Tool](https://www.tpc.org/tpc_documents_current_versions/download_programs/tools-download-request5.asp?bm_type=TPCX-AI&bm_vers=1.0.2&mode=CURRENT-ONLY)
* Documentation Link: [TPCxAI Documentation](https://www.tpc.org/tpc_documents_current_versions/pdf/tpcx-ai_v1.0.2.pdf)
## Setup & Instructions
1. Once Downloaded, in the root folder open file *setenv.sh* and find environment variable `TPCxAI_SCALE_FACTOR`.
2. Based on the required size, change the value of the Scale Factor. This value represents the size of the generated datasets across all the 10 Use Cases that TPCxAI supports (For more details on the use-cases, check the Documentation). 
    | Scale Factor  | Size  |
    | ------------- | ----- |
    | 1             | 1GB   |
    | 3             | 3GB   |
    | 10            | 10GB  |
    | 30            | 30GB  |
    | 100           | 100GB |
    | ...           | ...   |
    | 10,000        | 10TB  |
 > TPCxAI Supports Scale Factors in multiples of form `(1|3)*10^x` upto `10,000`. *(i.e.: 1, 3, 10, 30, 100, 300, ..., 10,000)*
3. Once the value is set, save and close the file.
4. Run the file `TPCx-AI_Benchmarkrun.sh`. It takes a while depending on the Scale Factor.
5. Once done, the generated datasets should be available at `[tool_root_dir]/output/data/`

