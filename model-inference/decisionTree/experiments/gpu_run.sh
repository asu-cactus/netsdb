python test_model.py -d higgs -m xgboost -f HummingbirdPytorchGPU --batch_size 100 --query_size 100
python test_model.py -d higgs -m xgboost -f HummingbirdPytorchGPU --batch_size 10000 --query_size 10000
python test_model.py -d higgs -m xgboost -f HummingbirdPytorchGPU --batch_size 1000000 --query_size 1000000
python test_model.py -d higgs -m xgboost -f HummingbirdTorchScriptGPU --batch_size 100 --query_size 100
python test_model.py -d higgs -m xgboost -f HummingbirdTorchScriptGPU --batch_size 10000 --query_size 10000
python test_model.py -d higgs -m xgboost -f HummingbirdTorchScriptGPU --batch_size 1000000 --query_size 1000000
python test_model.py -d higgs -m xgboost -f ONNXGPU --batch_size 100 --query_size 100
python test_model.py -d higgs -m xgboost -f ONNXGPU --batch_size 10000 --query_size 10000
python test_model.py -d higgs -m xgboost -f ONNXGPU --batch_size 1000000 --query_size 1000000
# python test_model.py -d higgs -m xgboost -f HummingbirdTVMGPU --batch_size 100 --query_size 100
# python test_model.py -d higgs -m xgboost -f HummingbirdTVMGPU --batch_size 10000 --query_size 10000
# python test_model.py -d higgs -m xgboost -f HummingbirdTVMGPU --batch_size 1000000 --query_size 1000000
python test_model.py -d higgs -m xgboost -f NvidiaFILGPU --batch_size 100 --query_size 100


python test_model.py -d higgs -m randomforest -f HummingbirdPytorchGPU --batch_size 100 --query_size 100
python test_model.py -d higgs -m randomforest -f HummingbirdPytorchGPU --batch_size 10000 --query_size 10000
python test_model.py -d higgs -m randomforest -f HummingbirdPytorchGPU --batch_size 1000000 --query_size 1000000
python test_model.py -d higgs -m randomforest -f HummingbirdTorchScriptGPU --batch_size 100 --query_size 100
python test_model.py -d higgs -m randomforest -f HummingbirdTorchScriptGPU --batch_size 10000 --query_size 10000
python test_model.py -d higgs -m randomforest -f HummingbirdTorchScriptGPU --batch_size 1000000 --query_size 1000000
python test_model.py -d higgs -m randomforest -f ONNXGPU --batch_size 100 --query_size 100
python test_model.py -d higgs -m randomforest -f ONNXGPU --batch_size 10000 --query_size 10000
python test_model.py -d higgs -m randomforest -f ONNXGPU --batch_size 1000000 --query_size 1000000
# python test_model.py -d higgs -m randomforest -f HummingbirdTVMGPU --batch_size 100 --query_size 100
# python test_model.py -d higgs -m randomforest -f HummingbirdTVMGPU --batch_size 10000 --query_size 10000
# python test_model.py -d higgs -m randomforest -f HummingbirdTVMGPU --batch_size 1000000 --query_size 1000000