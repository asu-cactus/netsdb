#! /bin/bash
DATASET=higgs

StringArray=(
	"python test_model.py -d year -m randomforest -f HummingbirdPytorchGPU -t 10 --batch_size 103069 --query_size 103069"
	"python test_model.py -d year -m randomforest -f HummingbirdTVMGPU -t 10 --batch_size 10000 --query_size 10000"
	"python test_model.py -d year -m randomforest -f NvidiaFILGPU -t 10 --batch_size 10000 --query_size 10000"
	"python test_model.py -d year -m xgboost -f HummingbirdPytorchGPU -t 10 --batch_size 103069 --query_size 103069"
	"python test_model.py -d year -m xgboost -f HummingbirdTVMGPU -t 10 --batch_size 10000 --query_size 10000"
	"python test_model.py -d year -m xgboost -f NvidiaFILGPU -t 10 --batch_size 100000 --query_size 100000"
	"python test_model.py -d year -m lightgbm -f HummingbirdPytorchGPU -t 10 --batch_size 103069 --query_size 103069"
	"python test_model.py -d year -m lightgbm -f HummingbirdTVMGPU -t 10 --batch_size 10000 --query_size 10000"
	"python test_model.py -d year -m lightgbm -f NvidiaFILGPU -t 10 --batch_size 10000 --query_size 10000"
	"python test_model.py -d year -m randomforest -f HummingbirdPytorchGPU -t 500 --batch_size 103069 --query_size 103069"
	"python test_model.py -d year -m randomforest -f HummingbirdTVMGPU -t 500 --batch_size 103069 --query_size 103069"
	"python test_model.py -d year -m randomforest -f NvidiaFILGPU -t 500 --batch_size 10000 --query_size 10000"
	"python test_model.py -d year -m xgboost -f HummingbirdPytorchGPU -t 500 --batch_size 103069 --query_size 103069"
	"python test_model.py -d year -m xgboost -f HummingbirdTVMGPU -t 500 --batch_size 103069 --query_size 103069"
	"python test_model.py -d year -m xgboost -f NvidiaFILGPU -t 500 --batch_size 103069 --query_size 103069"
	"python test_model.py -d year -m lightgbm -f HummingbirdPytorchGPU -t 500 --batch_size 103069 --query_size 103069"
	"python test_model.py -d year -m lightgbm -f HummingbirdTVMGPU -t 500 --batch_size 103069 --query_size 103069"
	"python test_model.py -d year -m lightgbm -f NvidiaFILGPU -t 500 --batch_size 10000 --query_size 10000"
	"python test_model.py -d year -m randomforest -f HummingbirdTVMGPU -t 1600 --batch_size 103069 --query_size 103069"
	"python test_model.py -d year -m randomforest -f NvidiaFILGPU -t 1600 --batch_size 10000 --query_size 10000"
	"python test_model.py -d year -m xgboost -f HummingbirdTVMGPU -t 1600 --batch_size 10000 --query_size 10000"
	"python test_model.py -d year -m xgboost -f NvidiaFILGPU -t 1600 --batch_size 103069 --query_size 103069"
	"python test_model.py -d year -m lightgbm -f HummingbirdTVMGPU -t 1600 --batch_size 10000 --query_size 10000"
	"python test_model.py -d year -m lightgbm -f NvidiaFILGPU -t 1600 --batch_size 10000 --query_size 10000"
	)
for command in "${StringArray[@]}";
	do
		GPU_OUTPUT_LOG="gpu_profiles/${command// /_}_GPU_LOG.txt"
        TEST_OUTPUT_FILE="gpu_profiles/${command// /_}_OUT_LOG.txt"
		FINAL_COMMAND="${command} > ${TEST_OUTPUT_FILE} &"
		echo ${GPU_OUTPUT_LOG}
		echo ${TEST_OUTPUT_FILE}
		echo ${FINAL_COMMAND}
        # nvidia-smi dmon -i 0 -s mu -d 1 -o TD > ${GPU_OUTPUT_LOG} &
		gpustat -i 0.5 > ${GPU_OUTPUT_LOG} &
        P1=$!
		eval ${FINAL_COMMAND}
		P2=$!
		echo $P1
		echo $P2
		wait $P2
		kill $P1
    done

sudo shutdown now -h
