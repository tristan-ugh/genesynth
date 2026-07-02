import os
os.environ["HSA_OVERRIDE_GFX_VERSION"] = "11.0.1"

# Sous WSL2, le profilage matériel rocprofiler requiert KFD qui est absent.
# Charger un stub vide évite un crash à l'import de PyTorch.
import ctypes
try:
    ctypes.CDLL('/opt/rocm/lib/librocprofiler_stub.so', mode=ctypes.RTLD_GLOBAL)
except Exception:
    pass

import torch

print(f"PyTorch version : {torch.__version__}")
print(f"ROCm disponible : {torch.cuda.is_available()}")
print(f"Nombre de GPUs  : {torch.cuda.device_count()}")

if torch.cuda.is_available():
    print(f"GPU             : {torch.cuda.get_device_name(0)}")
    x = torch.randn(1000, 1000).cuda()
    y = torch.matmul(x, x.T)
    print(f"Matmul GPU OK   : {y.shape}")
    print(f"Device          : {y.device}")
else:
    print("ATTENTION : GPU non detecte par PyTorch !")
