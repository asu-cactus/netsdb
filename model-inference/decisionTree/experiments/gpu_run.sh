#! /bin/bash
TREES="10"
DATASET="airline_classification"
MODELS="randomforest xgboost lightgbm"
FRAMEWORKS="HummingbirdPytorchGPU HummingbirdTorchScriptGPU ONNXGPU HummingbirdTVMGPU NvidiaFILGPU"

# Warmup
python test_model.py -d $DATASET -m xgboost -f Sklearn --batch_size 1000 --query_size 1000

echo "TESTS START HERE"
echo "*******************************8"

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
esac


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
                echo "python test_model.py -d $DATASET -m $MODEL -f $FRAMEWORK --batch_size $BATCHSIZE --query_size $QUERYSIZE --num_trees $TREES"
                python test_model.py -d $DATASET -m $MODEL -f $FRAMEWORK --batch_size $BATCHSIZE --query_size $QUERYSIZE --num_trees $TREES
                echo -e "\n\n"
            fi
        done
    done
done

# If you are running on a server, you can shut it down after the experiments are done
sudo shutdown now -h

# python test_model.py -d higgs -m xgboost -f HummingbirdPytorchGPU --batch_size 100 --query_size 100
# python test_model.py -d higgs -m xgboost -f HummingbirdPytorchGPU --batch_size 10000 --query_size 10000
# python test_model.py -d higgs -m xgboost -f HummingbirdPytorchGPU --batch_size 1000000 --query_size 1000000
# python test_model.py -d higgs -m xgboost -f HummingbirdTorchScriptGPU --batch_size 100 --query_size 100
# python test_model.py -d higgs -m xgboost -f HummingbirdTorchScriptGPU --batch_size 10000 --query_size 10000
# python test_model.py -d higgs -m xgboost -f HummingbirdTorchScriptGPU --batch_size 1000000 --query_size 1000000
# python test_model.py -d higgs -m xgboost -f ONNXGPU --batch_size 100 --query_size 100
# python test_model.py -d higgs -m xgboost -f ONNXGPU --batch_size 10000 --query_size 10000
# python test_model.py -d higgs -m xgboost -f ONNXGPU --batch_size 1000000 --query_size 1000000
# # python test_model.py -d higgs -m xgboost -f HummingbirdTVMGPU --batch_size 100 --query_size 100
# # python test_model.py -d higgs -m xgboost -f HummingbirdTVMGPU --batch_size 10000 --query_size 10000
# # python test_model.py -d higgs -m xgboost -f HummingbirdTVMGPU --batch_size 1000000 --query_size 1000000
# python test_model.py -d higgs -m xgboost -f NvidiaFILGPU --batch_size 100 --query_size 100


# python test_model.py -d higgs -m randomforest -f HummingbirdPytorchGPU --batch_size 100 --query_size 100
# python test_model.py -d higgs -m randomforest -f HummingbirdPytorchGPU --batch_size 10000 --query_size 10000
# python test_model.py -d higgs -m randomforest -f HummingbirdPytorchGPU --batch_size 1000000 --query_size 1000000
# python test_model.py -d higgs -m randomforest -f HummingbirdTorchScriptGPU --batch_size 100 --query_size 100
# python test_model.py -d higgs -m randomforest -f HummingbirdTorchScriptGPU --batch_size 10000 --query_size 10000
# python test_model.py -d higgs -m randomforest -f HummingbirdTorchScriptGPU --batch_size 1000000 --query_size 1000000
# python test_model.py -d higgs -m randomforest -f ONNXGPU --batch_size 100 --query_size 100
# python test_model.py -d higgs -m randomforest -f ONNXGPU --batch_size 10000 --query_size 10000
# python test_model.py -d higgs -m randomforest -f ONNXGPU --batch_size 1000000 --query_size 1000000
# # python test_model.py -d higgs -m randomforest -f HummingbirdTVMGPU --batch_size 100 --query_size 100
# # python test_model.py -d higgs -m randomforest -f HummingbirdTVMGPU --batch_size 10000 --query_size 10000
# # python test_model.py -d higgs -m randomforest -f HummingbirdTVMGPU --batch_size 1000000 --query_size 1000000