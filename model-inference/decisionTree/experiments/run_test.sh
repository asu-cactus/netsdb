#! /bin/bash
DATASET="tpcxai_fraud"
TREES="1600"
MODELS="randomforest xgboost lightgbm"
FRAMEWORKS="Sklearn ONNXCPU HummingbirdPytorchCPU HummingbirdTorchScriptCPU HummingbirdTVMCPU TreeLite Lleaves"

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
"criteo")
    TESTSIZE="51882752"
    BATCHSIZES="1000 10000 100000 1000000 10000000 51882752"
    ;;
"tpcxai_fraud")
    TESTSIZE="447622241"
    BATCHSIZES="1000 10000 100000 1000000 10000000 100000000 447622241"
    ;;
esac

python data_processing.py -d tpcxai_fraud -sf 100 -n 100

cat config_10.json > config.json
TREES="10"
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
            echo "python test_model.py -d $DATASET -m $MODEL -f $FRAMEWORK --batch_size $BATCHSIZE --query_size $QUERYSIZE --num_trees $TREES"
            python test_model.py -d $DATASET -m $MODEL -f $FRAMEWORK --batch_size $BATCHSIZE --query_size $QUERYSIZE --num_trees $TREES
            echo -e "\n\n"   
        done
    done
done

cat config_500.json > config.json
TREES="500"
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
            echo "python test_model.py -d $DATASET -m $MODEL -f $FRAMEWORK --batch_size $BATCHSIZE --query_size $QUERYSIZE --num_trees $TREES"
            python test_model.py -d $DATASET -m $MODEL -f $FRAMEWORK --batch_size $BATCHSIZE --query_size $QUERYSIZE --num_trees $TREES
            echo -e "\n\n"   
        done
    done
done

cat config_1600.json > config.json
TREES="1600"
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
            echo "python test_model.py -d $DATASET -m $MODEL -f $FRAMEWORK --batch_size $BATCHSIZE --query_size $QUERYSIZE --num_trees $TREES"
            python test_model.py -d $DATASET -m $MODEL -f $FRAMEWORK --batch_size $BATCHSIZE --query_size $QUERYSIZE --num_trees $TREES
            echo -e "\n\n"   
        done
    done
done

# If you are running on a server, you can shut it down after the experiments are done
sudo shutdown now -h
