call ..\..\python_venvs\digitsNN\Scripts\activate
python digitNN.py
python -m tf2onnx.convert --saved-model ./model_saved --output ../char_nn.onnx --opset 13
pause