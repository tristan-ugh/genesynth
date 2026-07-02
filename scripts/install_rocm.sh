#!/bin/bash
# ============================================================
# GeneSynth — Installation ROCm 6.4 via amdgpu-install
# Ubuntu 24.04 (WSL2) — RX 7800 XT (gfx1101)
# ============================================================
set -e

echo "==> [1/5] Suppression des conflits Ubuntu ROCm..."
sudo apt-get remove -y --purge rocminfo hipcc rocm-cmake 2>/dev/null || true
sudo apt-get autoremove -y 2>/dev/null || true

echo "==> [2/5] Suppression des anciens dépôts ROCm si présents..."
sudo rm -f /etc/apt/sources.list.d/rocm.list
sudo rm -f /etc/apt/sources.list.d/amdgpu.list
sudo rm -f /etc/apt/keyrings/rocm.gpg
sudo apt-get update -q

echo "==> [3/5] Téléchargement du script amdgpu-install 6.4..."
wget -q https://repo.radeon.com/amdgpu-install/6.4/ubuntu/noble/amdgpu-install_6.4.60400-1_all.deb \
    -O /tmp/amdgpu-install.deb

echo "==> [4/5] Installation du gestionnaire amdgpu-install..."
sudo apt-get install -y /tmp/amdgpu-install.deb
sudo apt-get update -q

echo "==> [5/5] Installation ROCm (userspace only, sans pilote kernel)..."
# --no-dkms = pas de module kernel (essentiel pour WSL2)
# --usecase=rocm = HIP + ROCm runtime + bibliothèques compute
sudo amdgpu-install -y --usecase=rocm --no-dkms

echo "==> Configuration utilisateur et variables d'environnement..."
sudo usermod -aG render,video "$USER"

if ! grep -q "ROCm" ~/.bashrc; then
    cat >> ~/.bashrc << 'EOF'

# ROCm
export PATH="/opt/rocm/bin:$PATH"
export LD_LIBRARY_PATH="/opt/rocm/lib:$LD_LIBRARY_PATH"
# Nécessaire pour RX 7800 XT (RDNA3 = gfx1101)
export HSA_OVERRIDE_GFX_VERSION=11.0.1
EOF
fi

echo ""
echo "============================================================"
echo "  Installation terminée !"
echo "  Lance : source ~/.bashrc && rocminfo | grep -A5 'Agent 2'"
echo "============================================================"
