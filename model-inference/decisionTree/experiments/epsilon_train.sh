echo "started"

python train_model.py -d epsilon -m randomforest -t 10
python train_model.py -d epsilon -m randomforest -t 500
python train_model.py -d epsilon -m randomforest -t 1600
python train_model.py -d epsilon -m xgboost -t 10
python train_model.py -d epsilon -m xgboost -t 500
python train_model.py -d epsilon -m xgboost -t 1600
python train_model.py -d epsilon -m lightgbm -t 10
python train_model.py -d epsilon -m lightgbm -t 500
python train_model.py -d epsilon -m lightgbm -t 1600
sudo shutdown now -h