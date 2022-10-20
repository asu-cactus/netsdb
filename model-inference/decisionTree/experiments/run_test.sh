#! /bin/bash
DATASET="higgs"
MODELS="randomforest xgboost lightgbm"
FRAMEWORKS="Sklearn TreeLite HummingbirdTVMCPU ONNXCPU Lleaves"

case $DATASET in
"higgs")
    TESTSIZE="2200000"
    BATCHSIZES="100 1000 10000 100000 2200000"
    ;;
"airline_classification" | "airline_regression")
    TESTSIZE="23013804"
    BATCHSIZES="100 1000 10000 100000 1000000 23013804"
    ;;
"fraud")
    TESTSIZE="56962"
    BATCHSIZES="100 1000 10000 56962"
    ;;
"year")
    TESTSIZE="103069"
    BATCHSIZES="100 1000 10000 100000 103069"
    ;;
"epsilon")
    TESTSIZE="100000"
    BATCHSIZES="100 1000 10000 100000"
    ;;
"bosch")
    TESTSIZE="236750"
    BATCHSIZES="100 1000 10000 236750"
    ;;
"covtype")
    TESTSIZE="116203"
    BATCHSIZES="100 1000 10000 236750"
    ;;
"tpcxai_fraud")
    TESTSIZE="131564160"
    BATCHSIZES="1000 10000 100000 1000000 10000000 100000000 131564160"
    ;;
esac


for MODEL in $MODELS
    do 
    for FRAMEWORK in $FRAMEWORKS
        do
        for BATCHSIZE in $BATCHSIZES
            do 
            if [ $FRAMEWORK == "TFDF" ]
                then 
                    QUERYSIZE=$TESTSIZE
                else
                    QUERYSIZE=$BATCHSIZE
            fi
            echo "python test_model.py -d $DATASET -m $MODEL -f $FRAMEWORK --batch_size $BATCHSIZE --query_size $QUERYSIZE"
            python test_model.py -d $DATASET -m $MODEL -f $FRAMEWORK --batch_size $BATCHSIZE --query_size $QUERYSIZE
            echo -e "\n\n"   
        done
    done
done

# If you are running on a server, you can shut it down after the experiments are done
sudo shutdown now -h