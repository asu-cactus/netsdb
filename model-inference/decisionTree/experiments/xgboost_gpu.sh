echo "started"

python test_model.py -d higgs -m randomforest -f XGBoostGPU --batch_size 100 --query_size 100 -t 10
python test_model.py -d higgs -m randomforest -f XGBoostGPU --batch_size 1000 --query_size 1000 -t 10
python test_model.py -d higgs -m randomforest -f XGBoostGPU --batch_size 10000 --query_size 10000 -t 10
python test_model.py -d higgs -m randomforest -f XGBoostGPU --batch_size 100000 --query_size 100000 -t 10
python test_model.py -d higgs -m randomforest -f XGBoostGPU --batch_size 2200000 --query_size 2200000 -t 10
python test_model.py -d higgs -m randomforest -f XGBoostGPU --batch_size 100 --query_size 100 -t 500
python test_model.py -d higgs -m randomforest -f XGBoostGPU --batch_size 1000 --query_size 1000 -t 500
python test_model.py -d higgs -m randomforest -f XGBoostGPU --batch_size 10000 --query_size 10000 -t 500
python test_model.py -d higgs -m randomforest -f XGBoostGPU --batch_size 100000 --query_size 100000 -t 500
python test_model.py -d higgs -m randomforest -f XGBoostGPU --batch_size 2200000 --query_size 2200000 -t 500
python test_model.py -d higgs -m randomforest -f XGBoostGPU --batch_size 100 --query_size 100 -t 1600
python test_model.py -d higgs -m randomforest -f XGBoostGPU --batch_size 1000 --query_size 1000 -t 1600
python test_model.py -d higgs -m randomforest -f XGBoostGPU --batch_size 10000 --query_size 10000 -t 1600
python test_model.py -d higgs -m randomforest -f XGBoostGPU --batch_size 100000 --query_size 100000 -t 1600
python test_model.py -d higgs -m randomforest -f XGBoostGPU --batch_size 2200000 --query_size 2200000 -t 1600
sudo shutdown now -h