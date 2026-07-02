#!/bin/bash
# ============================================================
# GeneSynth — Setup environnement Python (venv + PyTorch ROCm)
# Ubuntu 24.04 (WSL2) — RX 7800 XT (gfx1101 / ROCm 6.x)
# ============================================================
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
VENV_DIR="$PROJECT_ROOT/python/.venv"

echo "==> [1/4] Création du venv Python dans python/.venv ..."
python3 -m venv "$VENV_DIR"
source "$VENV_DIR/bin/activate"

echo "==> [2/4] Mise à jour de pip..."
pip install --quiet --upgrade pip

echo "==> [3/4] Installation de PyTorch avec backend ROCm 6.3..."
# ROCm 6.3 wheel = compatible ROCm 6.4 installé
pip install torch torchvision torchaudio --index-url https://download.pytorch.org/whl/rocm6.3

echo "==> [4/4] Installation des autres dépendances ML..."
pip install \
    numpy \
    scipy \
    librosa \
    soundfile \
    scikit-learn \
    matplotlib \
    tqdm \
    onnx \
    onnxruntime \
    pyaml \
    psutil

echo ""
echo "============================================================"
echo "  Venv créé dans : python/.venv"
echo "  Pour l'activer : source python/.venv/bin/activate"
echo "============================================================"
