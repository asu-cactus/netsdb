echo "started"

python train_model.py -d epsilon -m randomforest -t 10
python train_model.py -d epsilon -m randomforest -t 500
python train_model.py -d epsilon -m randomforest -t 1600

sudo shutdown now -h