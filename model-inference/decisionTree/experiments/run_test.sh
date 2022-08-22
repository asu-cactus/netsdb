#! /bin/bash
DATASET="year"
MODELS="randomforest xgboost"
# TESTSIZE="1000"
# BATCHSIZES="100 1000"
FRAMEWORKS="Sklearn TreeLite HummingbirdPytorchCPU HummingbirdTorchScriptCPU HummingbirdTVMCPU TFDF ONNXCPU"
# FRAMEWORKS="Sklearn TreeLite HummingbirdPytorchCPU HummingbirdTorchScriptCPU HummingbirdTVMCPU ONNXCPU"

# Warmup
python test_model.py -d $DATASET -m xgboost -f Sklearn --batch_size 1000 --query_size 1000

switch $DATASET
case "higgs"
    TESTSIZE="2200000"
    BATCHSIZES="1 10 100 1000 10000 100000 2200000"
case "airline_classification"
    TESTSIZE="23013804"
    BATCHSIZES="1 10 100 1000 10000 100000 1000000 23013804"
case "airline_regression"
    TESTSIZE="23013804"
    BATCHSIZES="1 10 100 1000 10000 100000 1000000 23013804"
case "fraud"
    TESTSIZE="56962"
    BATCHSIZES="1 10 100 1000 10000 56962"
case "year"
    TESTSIZE="103069"
    BATCHSIZES="1 10 100 1000 10000 100000 103069"
case "epsilon"
    TESTSIZE="100000"
    BATCHSIZES="1 10 100 1000 10000 100000"
endswitch


for MODEL in $MODELS
    do 
    for FRAMEWORK in $FRAMEWORKS
        do
        for BATCHSIZE in $BATCHSIZES
            do 
            if [ "$MODEL $FRAMEWORK" != "randomforest TreeLite" ]
                then

                if [ $FRAMEWORK == "TFDF" ]
                then 
                    QUERYSIZE=$TESTSIZE
                else
                    QUERYSIZE=$BATCHSIZE
                fi
                echo "python test_model.py -d $DATASET -m $MODEL -f $FRAMEWORK --batch_size $BATCHSIZE --query_size $QUERYSIZE"
                python test_model.py -d $DATASET -m $MODEL -f $FRAMEWORK --batch_size $BATCHSIZE --query_size $QUERYSIZE
                echo -e "\n\n"
            fi
        done
    done
done

# If you are running on a server, you can shut it down after the experiments are done
# sudo showdown now -h