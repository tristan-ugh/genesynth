#!/bin/bash
# Configuration finale des variables d'environnement ROCm/librocdxg dans ~/.bashrc

# Nettoyer les anciennes entrées
sed -i '/# ROCm/,/HSA_OVERRIDE_GFX_VERSION/d' ~/.bashrc 2>/dev/null || true

# Ajouter la nouvelle config complète
cat >> ~/.bashrc << 'EOF'

# ROCm + librocdxg (WSL2 - RX 7800 XT)
export PATH="/opt/rocm/bin:$PATH"
export LD_LIBRARY_PATH="/opt/rocm/lib:/usr/lib/wsl/lib:$LD_LIBRARY_PATH"
export HSA_ENABLE_DXG_DETECTION=1
export HSA_OVERRIDE_GFX_VERSION=11.0.1
EOF

echo "Variables d'environnement ajoutées dans ~/.bashrc"
