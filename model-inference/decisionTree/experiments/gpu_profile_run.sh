higgs_10

python test_model.py -d higgs -m randomforest -f HummingbirdPytorchGPU -t 10 --batch_size 2200000 --query_size 2200000
python test_model.py -d higgs -m randomforest -f HummingbirdTVMGPU -t 10 --batch_size 2200000 --query_size 2200000
python test_model.py -d higgs -m xgboost -f HummingbirdPytorchGPU -t 10 --batch_size 2200000 --query_size 2200000
python test_model.py -d higgs -m xgboost -f HummingbirdTVMGPU -t 10 --batch_size 100000 --query_size 100000
python test_model.py -d higgs -m lightgbm -f HummingbirdPytorchGPU -t 10 --batch_size 2200000 --query_size 2200000
python test_model.py -d higgs -m lightgbm -f HummingbirdTVMGPU -t 10 --batch_size 100000 --query_size 100000

higgs_500

python test_model.py -d higgs -m randomforest -f HummingbirdTVMGPU -t 500 --batch_size 100000 --query_size 100000
python test_model.py -d higgs -m randomforest -f NvidiaFILGPU -t 500 --batch_size 100000 --query_size 100000
python test_model.py -d higgs -m xgboost -f HummingbirdTVMGPU -t 500 --batch_size 100000 --query_size 100000
python test_model.py -d higgs -m xgboost -f NvidiaFILGPU -t 500 --batch_size 2200000 --query_size 2200000
python test_model.py -d higgs -m lightgbm -f HummingbirdTVMGPU -t 500 --batch_size 100000 --query_size 100000
python test_model.py -d higgs -m lightgbm -f NvidiaFILGPU -t 500 --batch_size 100000 --query_size 100000

higgs_1600

python test_model.py -d higgs -m randomforest -f HummingbirdTVMGPU -t 1600 --batch_size 100000 --query_size 100000
python test_model.py -d higgs -m randomforest -f NvidiaFILGPU -t 1600 --batch_size 100000 --query_size 100000
python test_model.py -d higgs -m xgboost -f HummingbirdTVMGPU -t 1600 --batch_size 100000 --query_size 100000
python test_model.py -d higgs -m xgboost -f NvidiaFILGPU -t 1600 --batch_size 10000 --query_size 10000
python test_model.py -d higgs -m lightgbm -f HummingbirdTVMGPU -t 1600 --batch_size 10000 --query_size 10000
python test_model.py -d higgs -m lightgbm -f NvidiaFILGPU -t 1600 --batch_size 10000 --query_size 10000


airline_10

python test_model.py -d airline -m randomforest -f HummingbirdPytorchGPU -t 10 --batch_size 23013804 --query_size 23013804
python test_model.py -d airline -m randomforest -f HummingbirdTorchScriptGPU -t 10 --batch_size 23013804 --query_size 23013804
python test_model.py -d airline -m randomforest -f HummingbirdTVMGPU -t 10 --batch_size 23013804 --query_size 23013804
python test_model.py -d airline -m xgboost -f HummingbirdPytorchGPU -t 10 --batch_size 23013804 --query_size 23013804
python test_model.py -d airline -m xgboost -f HummingbirdTorchScriptGPU -t 10 --batch_size 23013804 --query_size 23013804
python test_model.py -d airline -m xgboost -f HummingbirdTVMGPU -t 10 --batch_size 23013804 --query_size 23013804
python test_model.py -d airline -m lightgbm -f HummingbirdPytorchGPU -t 10 --batch_size 23013804 --query_size 23013804
python test_model.py -d airline -m lightgbm -f HummingbirdTorchScriptGPU -t 10 --batch_size 23013804 --query_size 23013804
python test_model.py -d airline -m lightgbm -f HummingbirdTVMGPU -t 10 --batch_size 23013804 --query_size 23013804

airline_500

python test_model.py -d airline -m randomforest -f HummingbirdTVMGPU -t 500 --batch_size 100000 --query_size 100000
python test_model.py -d airline -m randomforest -f NvidiaFILGPU -t 500 --batch_size 100000 --query_size 100000
python test_model.py -d airline -m xgboost -f HummingbirdTVMGPU -t 500 --batch_size 100000 --query_size 100000
python test_model.py -d airline -m xgboost -f NvidiaFILGPU -t 500 --batch_size 100000 --query_size 100000
python test_model.py -d airline -m xgboost -f XGBoostGPU -t 500 --batch_size 23013804 --query_size 23013804
python test_model.py -d airline -m lightgbm -f HummingbirdTVMGPU -t 500 --batch_size 100000 --query_size 100000
python test_model.py -d airline -m lightgbm -f NvidiaFILGPU -t 500 --batch_size 100000 --query_size 100000

airline_1600

python test_model.py -d airline -m randomforest -f HummingbirdTVMGPU -t 1600 --batch_size 10000 --query_size 10000
python test_model.py -d airline -m randomforest -f NvidiaFILGPU -t 1600 --batch_size 10000 --query_size 10000
python test_model.py -d airline -m xgboost -f HummingbirdTVMGPU -t 1600 --batch_size 100000 --query_size 100000
python test_model.py -d airline -m xgboost -f NvidiaFILGPU -t 1600 --batch_size 100000 --query_size 100000
python test_model.py -d airline -m xgboost -f XGBoostGPU -t 1600 --batch_size 23013804 --query_size 23013804
python test_model.py -d airline -m lightgbm -f HummingbirdTVMGPU -t 1600 --batch_size 100000 --query_size 100000
python test_model.py -d airline -m lightgbm -f NvidiaFILGPU -t 1600 --batch_size 10000 --query_size 10000

fraud_10

python test_model.py -d fraud -m randomforest -f HummingbirdPytorchGPU -t 10 --batch_size 56962 --query_size 56962
python test_model.py -d fraud -m randomforest -f HummingbirdTVMGPU -t 10 --batch_size 56962 --query_size 56962
python test_model.py -d fraud -m randomforest -f NvidiaFILGPU -t 10 --batch_size 56962 --query_size 56962
python test_model.py -d fraud -m xgboost -f HummingbirdPytorchGPU -t 10 --batch_size 56962 --query_size 56962
python test_model.py -d fraud -m xgboost -f HummingbirdTVMGPU -t 10 --batch_size 56962 --query_size 56962
python test_model.py -d fraud -m xgboost -f NvidiaFILGPU -t 10 --batch_size 56962 --query_size 56962
python test_model.py -d fraud -m lightgbm -f HummingbirdPytorchGPU -t 10 --batch_size 56962 --query_size 56962
python test_model.py -d fraud -m lightgbm -f HummingbirdTVMGPU -t 10 --batch_size 56962 --query_size 56962
python test_model.py -d fraud -m lightgbm -f NvidiaFILGPU -t 10 --batch_size 10000 --query_size 10000

fraud_500

python test_model.py -d fraud -m randomforest -f HummingbirdPytorchGPU -t 500 --batch_size 56962 --query_size 56962
python test_model.py -d fraud -m randomforest -f HummingbirdTVMGPU -t 500 --batch_size 56962 --query_size 56962
python test_model.py -d fraud -m randomforest -f NvidiaFILGPU -t 500 --batch_size 10000 --query_size 10000
python test_model.py -d fraud -m xgboost -f HummingbirdPytorchGPU -t 500 --batch_size 56962 --query_size 56962
python test_model.py -d fraud -m xgboost -f HummingbirdTVMGPU -t 500 --batch_size 56962 --query_size 56962
python test_model.py -d fraud -m xgboost -f NvidiaFILGPU -t 500 --batch_size 56962 --query_size 56962
python test_model.py -d fraud -m lightgbm -f HummingbirdPytorchGPU -t 500 --batch_size 56962 --query_size 56962
python test_model.py -d fraud -m lightgbm -f HummingbirdTVMGPU -t 500 --batch_size 56962 --query_size 56962
python test_model.py -d fraud -m lightgbm -f NvidiaFILGPU -t 500 --batch_size 10000 --query_size 10000

fraud_1600

python test_model.py -d fraud -m randomforest -f HummingbirdTVMGPU -t 1600 --batch_size 56962 --query_size 56962
python test_model.py -d fraud -m randomforest -f NvidiaFILGPU -t 1600 --batch_size 56962 --query_size 56962
python test_model.py -d fraud -m xgboost -f HummingbirdTVMGPU -t 1600 --batch_size 56962 --query_size 56962
python test_model.py -d fraud -m xgboost -f NvidiaFILGPU -t 1600 --batch_size 56962 --query_size 56962
python test_model.py -d fraud -m lightgbm -f HummingbirdTVMGPU -t 1600 --batch_size 56962 --query_size 56962
python test_model.py -d fraud -m lightgbm -f NvidiaFILGPU -t 1600 --batch_size 56962 --query_size 56962

bosch_10

python test_model.py -d bosch -m xgboost -f HummingbirdPytorchGPU -t 10 --batch_size 236750 --query_size 236750
python test_model.py -d bosch -m xgboost -f HummingbirdTVMGPU -t 10 --batch_size 1000 --query_size 1000
python test_model.py -d bosch -m xgboost -f NvidiaFILGPU -t 10 --batch_size 236750 --query_size 236750
python test_model.py -d bosch -m lightgbm -f HummingbirdPytorchGPU -t 10 --batch_size 236750 --query_size 236750
python test_model.py -d bosch -m lightgbm -f HummingbirdTVMGPU -t 10 --batch_size 1000 --query_size 1000

bosch_500

python test_model.py -d bosch -m xgboost -f HummingbirdPytorchGPU -t 500 --batch_size 236750 --query_size 236750
python test_model.py -d bosch -m xgboost -f HummingbirdTVMGPU -t 500 --batch_size 1000 --query_size 1000
python test_model.py -d bosch -m xgboost -f NvidiaFILGPU -t 500 --batch_size 236750 --query_size 236750
python test_model.py -d bosch -m lightgbm -f HummingbirdPytorchGPU -t 500 --batch_size 236750 --query_size 236750
python test_model.py -d bosch -m lightgbm -f HummingbirdTVMGPU -t 500 --batch_size 10000 --query_size 10000

bosch_1600

python test_model.py -d bosch -m xgboost -f HummingbirdTVMGPU -t 1600 --batch_size 1000 --query_size 1000
python test_model.py -d bosch -m xgboost -f NvidiaFILGPU -t 1600 --batch_size 236750 --query_size 236750
python test_model.py -d bosch -m lightgbm -f HummingbirdTVMGPU -t 1600 --batch_size 1000 --query_size 1000
python test_model.py -d bosch -m lightgbm -f NvidiaFILGPU -t 1600 --batch_size 1000 --query_size 1000

year_10

python test_model.py -d year -m randomforest -f HummingbirdPytorchGPU -t 10 --batch_size 103069 --query_size 103069
python test_model.py -d year -m randomforest -f HummingbirdTVMGPU -t 10 --batch_size 10000 --query_size 10000
python test_model.py -d year -m randomforest -f NvidiaFILGPU -t 10 --batch_size 10000 --query_size 10000
python test_model.py -d year -m xgboost -f HummingbirdPytorchGPU -t 10 --batch_size 103069 --query_size 103069
python test_model.py -d year -m xgboost -f HummingbirdTVMGPU -t 10 --batch_size 10000 --query_size 10000
python test_model.py -d year -m xgboost -f NvidiaFILGPU -t 10 --batch_size 100000 --query_size 100000
python test_model.py -d year -m lightgbm -f HummingbirdPytorchGPU -t 10 --batch_size 103069 --query_size 103069
python test_model.py -d year -m lightgbm -f HummingbirdTVMGPU -t 10 --batch_size 10000 --query_size 10000
python test_model.py -d year -m lightgbm -f NvidiaFILGPU -t 10 --batch_size 10000 --query_size 10000

year_500

python test_model.py -d year -m randomforest -f HummingbirdPytorchGPU -t 500 --batch_size 103069 --query_size 103069
python test_model.py -d year -m randomforest -f HummingbirdTVMGPU -t 500 --batch_size 103069 --query_size 103069
python test_model.py -d year -m randomforest -f NvidiaFILGPU -t 500 --batch_size 10000 --query_size 10000
python test_model.py -d year -m xgboost -f HummingbirdPytorchGPU -t 500 --batch_size 103069 --query_size 103069
python test_model.py -d year -m xgboost -f HummingbirdTVMGPU -t 500 --batch_size 103069 --query_size 103069
python test_model.py -d year -m xgboost -f NvidiaFILGPU -t 500 --batch_size 103069 --query_size 103069
python test_model.py -d year -m lightgbm -f HummingbirdPytorchGPU -t 500 --batch_size 103069 --query_size 103069
python test_model.py -d year -m lightgbm -f HummingbirdTVMGPU -t 500 --batch_size 103069 --query_size 103069
python test_model.py -d year -m lightgbm -f NvidiaFILGPU -t 500 --batch_size 10000 --query_size 10000

year_1600

python test_model.py -d year -m randomforest -f HummingbirdTVMGPU -t 1600 --batch_size 103069 --query_size 103069
python test_model.py -d year -m randomforest -f NvidiaFILGPU -t 1600 --batch_size 10000 --query_size 10000
python test_model.py -d year -m xgboost -f HummingbirdTVMGPU -t 1600 --batch_size 10000 --query_size 10000
python test_model.py -d year -m xgboost -f NvidiaFILGPU -t 1600 --batch_size 103069 --query_size 103069
python test_model.py -d year -m lightgbm -f HummingbirdTVMGPU -t 1600 --batch_size 10000 --query_size 10000
python test_model.py -d year -m lightgbm -f NvidiaFILGPU -t 1600 --batch_size 10000 --query_size 10000

epsilon_10

python test_model.py -d epsilon -m randomforest -f HummingbirdPytorchGPU -t 10 --batch_size 100000 --query_size 100000
python test_model.py -d epsilon -m randomforest -f HummingbirdTVMGPU -t 10 --batch_size 1000 --query_size 1000
python test_model.py -d epsilon -m xgboost -f HummingbirdPytorchGPU -t 10 --batch_size 100000 --query_size 100000
python test_model.py -d epsilon -m xgboost -f HummingbirdTVMGPU -t 10 --batch_size 100000 --query_size 100000
python test_model.py -d epsilon -m lightgbm -f HummingbirdPytorchGPU -t 10 --batch_size 100000 --query_size 100000
python test_model.py -d epsilon -m lightgbm -f HummingbirdTVMGPU -t 10 --batch_size 1000 --query_size 1000

epsilon_500

python test_model.py -d epsilon -m randomforest -f HummingbirdPytorchGPU -t 500 --batch_size 100000 --query_size 100000
python test_model.py -d epsilon -m randomforest -f HummingbirdTVMGPU -t 500 --batch_size 100000 --query_size 100000
python test_model.py -d epsilon -m xgboost -f HummingbirdPytorchGPU -t 500 --batch_size 100000 --query_size 100000
python test_model.py -d epsilon -m xgboost -f HummingbirdTVMGPU -t 500 --batch_size 100 --query_size 100
python test_model.py -d epsilon -m xgboost -f NvidiaFILGPU -t 500 --batch_size 100000 --query_size 100000
python test_model.py -d epsilon -m lightgbm -f HummingbirdPytorchGPU -t 500 --batch_size 100000 --query_size 100000
python test_model.py -d epsilon -m lightgbm -f HummingbirdTVMGPU -t 500 --batch_size 100000 --query_size 100000
python test_model.py -d epsilon -m lightgbm -f NvidiaFILGPU -t 500 --batch_size 1000 --query_size 1000

epsilon_1600

python test_model.py -d epsilon -m randomforest -f HummingbirdPytorchGPU -t 1600 --batch_size 100000 --query_size 100000
python test_model.py -d epsilon -m randomforest -f HummingbirdTVMGPU -t 1600 --batch_size 10000 --query_size 10000
python test_model.py -d epsilon -m xgboost -f HummingbirdPytorchGPU -t 1600 --batch_size 100000 --query_size 100000
python test_model.py -d epsilon -m xgboost -f HummingbirdTVMGPU -t 1600 --batch_size 10000 --query_size 10000
python test_model.py -d epsilon -m xgboost -f NvidiaFILGPU -t 1600 --batch_size 1000 --query_size 1000
python test_model.py -d epsilon -m lightgbm -f HummingbirdPytorchGPU -t 1600 --batch_size 100000 --query_size 100000
python test_model.py -d epsilon -m lightgbm -f NvidiaFILGPU -t 1600 --batch_size 1000 --query_size 1000

tpcxai_fraud_10

python test_model.py -d tpcxai_fraud -m randomforest -f HummingbirdPytorchGPU -t 10 --batch_size 1000000 --query_size 1000000
python test_model.py -d tpcxai_fraud -m randomforest -f HummingbirdTVMGPU -t 10 --batch_size 1000000 --query_size 1000000
python test_model.py -d tpcxai_fraud -m xgboost -f HummingbirdPytorchGPU -t 10 --batch_size 1000000 --query_size 1000000
python test_model.py -d tpcxai_fraud -m xgboost -f HummingbirdTorchScriptGPU -t 10 --batch_size 131564160 --query_size 131564160
python test_model.py -d tpcxai_fraud -m lightgbm -f HummingbirdPytorchGPU -t 10 --batch_size 1000000 --query_size 1000000
python test_model.py -d tpcxai_fraud -m lightgbm -f HummingbirdTorchScriptGPU -t 10 --batch_size 131564160 --query_size 131564160

tpcxai_fraud_500

python test_model.py -d tpcxai_fraud -m randomforest -f HummingbirdTVMGPU -t 500 --batch_size 100000 --query_size 100000
python test_model.py -d tpcxai_fraud -m randomforest -f NvidiaFILGPU -t 500 --batch_size 100000 --query_size 100000
python test_model.py -d tpcxai_fraud -m xgboost -f HummingbirdTVMGPU -t 500 --batch_size 100000 --query_size 100000
python test_model.py -d tpcxai_fraud -m xgboost -f NvidiaFILGPU -t 500 --batch_size 100000 --query_size 100000
python test_model.py -d tpcxai_fraud -m xgboost -f XGBoostGPU -t 500 --batch_size 131564160 --query_size 131564160
python test_model.py -d tpcxai_fraud -m lightgbm -f HummingbirdTVMGPU -t 500 --batch_size 100000 --query_size 100000
python test_model.py -d tpcxai_fraud -m lightgbm -f NvidiaFILGPU -t 500 --batch_size 100000 --query_size 100000

tpcxai_fraud_1600

python test_model.py -d tpcxai_fraud -m randomforest -f HummingbirdTVMGPU -t 1600 --batch_size 10000 --query_size 10000
python test_model.py -d tpcxai_fraud -m randomforest -f NvidiaFILGPU -t 1600 --batch_size 100000 --query_size 100000
python test_model.py -d tpcxai_fraud -m xgboost -f HummingbirdTVMGPU -t 1600 --batch_size 10000 --query_size 10000
python test_model.py -d tpcxai_fraud -m xgboost -f NvidiaFILGPU -t 1600 --batch_size 100000 --query_size 100000
python test_model.py -d tpcxai_fraud -m lightgbm -f HummingbirdTVMGPU -t 1600 --batch_size 10000 --query_size 10000
python test_model.py -d tpcxai_fraud -m lightgbm -f NvidiaFILGPU -t 1600 --batch_size 10000 --query_size 10000
