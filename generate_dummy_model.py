import ctypes
try:
    # Charge le stub dans l'espace global du processus pour corriger le crash ROCm AMD
    ctypes.CDLL('/opt/rocm/lib/librocprofiler_stub.so', mode=ctypes.RTLD_GLOBAL)
except Exception:
    pass

import torch
import torch.nn as nn
import onnx

class DummyModel(nn.Module):
    def __init__(self):
        super(DummyModel, self).__init__()
        # Input shape expected: [Batch, Frames, 128]
        # We will average over Frames, then project to 41 output parameters
        self.fc = nn.Linear(128, 41)
        
    def forward(self, x):
        # x is [B, F, Mels]
        x_pooled = x.mean(dim=1) # [B, Mels]
        out = self.fc(x_pooled) # [B, 41]
        out = torch.sigmoid(out) # To get values in [0.0, 1.0]
        return out

model = DummyModel()
model.eval()

# Create a dummy input [Batch=1, Frames=86, Mels=128]
dummy_input = torch.randn(1, 86, 128)

torch.onnx.export(
    model, 
    dummy_input, 
    "dummy_model.onnx", 
    export_params=True,
    opset_version=14,
    input_names=['input'], 
    output_names=['output'],
    dynamic_axes={'input': {1: 'frames'}} # Frames dimension is dynamic
)

print("Exported dummy_model.onnx")
