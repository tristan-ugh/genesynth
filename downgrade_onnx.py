import onnx
import sys

model = onnx.load("dummy_model.onnx")
# Downgrade IR version to 8 (which is supported by ONNX Runtime 1.10+)
model.ir_version = 8
onnx.save(model, "dummy_model_v8.onnx")
print("Downgraded IR version to 8")
