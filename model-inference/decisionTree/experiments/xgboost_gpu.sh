python test_model.py -d higgs -m xgboost -f XGBoostGPU --batch_size 1 --query_size 1
python test_model.py -d higgs -m xgboost -f XGBoostGPU --batch_size 10 --query_size 10
python test_model.py -d higgs -m xgboost -f XGBoostGPU --batch_size 100 --query_size 100
python test_model.py -d higgs -m xgboost -f XGBoostGPU --batch_size 1000 --query_size 1000
python test_model.py -d higgs -m xgboost -f XGBoostGPU --batch_size 10000 --query_size 10000
python test_model.py -d higgs -m xgboost -f XGBoostGPU --batch_size 100000 --query_size 100000
python test_model.py -d higgs -m xgboost -f XGBoostGPU --batch_size 2200000 --query_size 2200000

sudo shutdown now -h