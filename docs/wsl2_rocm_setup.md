# Guide d'installation et d'architecture : ROCm & PyTorch (AMD RX 7800 XT) sous WSL2

Ce document détaille l'architecture, les défis techniques et la solution mise en œuvre pour faire fonctionner **PyTorch** avec accélération GPU matérielle sur une carte **AMD Radeon RX 7800 XT** sous **Windows Subsystem for Linux (WSL2)** dans le cadre du projet **GeneSynth**.

---

## 1. Vue d'ensemble de l'architecture

Faire tourner du calcul lourd (Deep Learning) sur un GPU AMD sous Windows via WSL2 repose sur une pile logicielle moderne qui a radicalement changé récemment.

```mermaid
graph TD
    subgraph Windows Host (Matériel & Pilote)
        A[AMD Radeon RX 7800 XT RDNA3] -->|Pilote Windows Adrenalin v32.0+| B[Interface DirectX Graphics DXG]
    end

    subgraph WSL2 (Linux Ubuntu 24.04)
        B -->|Mappage noyau WSL| C[/dev/dxg]
        C --> D[librocdxg.so v1.2.0]
        D -->|Pont d'appels compute| E[HSA Runtime /opt/rocm/lib/libhsa-runtime64.so]
        E --> F[PyTorch 2.12.1+rocm7.2 dans venv]
        G[Stub C ctypes] -.->|Mute rocprofiler| F
    end
```

---

## 2. Les Défis Techniques & Leurs Résolutions

### Défi n°1 : La disparition de `/dev/kfd` sous WSL2
* **Problème** : Historiquement, ROCm sous Linux communique avec le GPU via le pilote noyau KFD (exposé sous `/dev/kfd`). Or, WSL2 ne gère pas les pilotes de noyau Linux natifs pour les GPU AMD.
* **Résolution** : AMD a développé **ROCDXG** (`librocdxg`). Ce composant utilise l'interface DirectX de Windows (`/dev/dxg`) déjà intégrée dans le noyau WSL2 de Microsoft pour faire passerelle. C'est la méthode officiellement supportée en 2026.

### Défi n°2 : La compatibilité RDNA3 (`gfx1101`)
* **Problème** : PyTorch compile ses binaires ROCm principalement pour les architectures serveurs (ex: `gfx1100` pour la RX 7900 XTX). La RX 7800 XT utilise l'architecture `gfx1101` (RDNA3 grand public), qui n'est pas ciblée par défaut par les compilations standards de PyTorch.
* **Résolution** : Utiliser la variable d'environnement :
  ```bash
  export HSA_OVERRIDE_GFX_VERSION=11.0.1
  ```
  Cette variable indique au runtime ROCm de traiter la RX 7800 XT comme une carte compatible RDNA3 standard, évitant les erreurs de GPU non reconnu.

### Défi n°3 : Le crash de `rocprofiler` sur WSL2 (PyTorch 2.12+)
* **Problème** : À partir de PyTorch 2.12, le profileur de performance `rocprofiler` est lié statiquement au package. À l'initialisation, il interroge le chemin de topologie `/sys/class/kfd/...` pour compter les agents de profilage. Sous WSL2 (qui utilise `/dev/dxg`), ce dossier n'existe pas. Constatant une incohérence (0 agent de profilage pour 2 agents de calcul HSA), PyTorch déclenche un signal d'avortement (`Aborted / core dumped`).
* **Résolution** : Créer un **Stub C** (une bibliothèque vide) qui redéfinit les fonctions d'enregistrement de `rocprofiler` et les neutralise. Ce stub est chargé dynamiquement par Python via `ctypes` avant l'import de `torch`, évitant ainsi d'avoir à modifier les fichiers internes du venv.

---

## 3. Guide pas-à-pas pour recréer l'environnement

### Étape 1 : Prérequis système (Windows)
1. Installer la dernière version de **AMD Software: Adrenalin Edition** (v32.0.x ou supérieure).
2. Mettre à jour WSL2 dans PowerShell (en mode administrateur) :
   ```powershell
   wsl --update
   ```
3. S'assurer que le fichier `/dev/dxg` est bien visible dans WSL :
   ```bash
   ls -la /dev/dxg
   ```

### Étape 2 : Installation de ROCm & de librocdxg (Linux WSL)
1. **Ajouter le dépôt ROCm officiel (ici ROCm 6.4/7.2)** :
   ```bash
   sudo mkdir -p /etc/apt/keyrings
   wget -q -O - https://repo.radeon.com/rocm/rocm.gpg.key | gpg --dearmor | sudo tee /etc/apt/keyrings/rocm.gpg > /dev/null
   echo "deb [arch=amd64 signed-by=/etc/apt/keyrings/rocm.gpg] https://repo.radeon.com/rocm/apt/6.4 noble main" | sudo tee /etc/apt/sources.list.d/rocm.list
   ```
2. **Installer le runtime** :
   ```bash
   sudo apt update
   sudo apt install -y rocm-hip-runtime rocminfo
   ```
3. **Installer le pont `librocdxg`** :
   Télécharger le paquet `.deb` correspondant à ta version depuis le dépôt officiel [ROCm/librocdxg](https://github.com/ROCm/librocdxg/releases) et l'installer :
   ```bash
   sudo dpkg -i rocdxg-roct_1.2.0_amd64.deb
   ```

### Étape 3 : Compilation du Stub `rocprofiler`
1. Écrire le code source du stub (`rocprofiler_stub.c`) :
   ```c
   #define _GNU_SOURCE
   #include <stdio.h>
   #include <stdint.h>

   int rocprofiler_register_library_api_table(const char* name, void* api_table) {
       return 0; // Succès silencieux
   }

   int rocprofiler_set_api_table(const char* name, uint64_t lib_version, void* api_table) {
       return 0; // Succès silencieux
   }
   ```
2. Compiler sous forme de bibliothèque partagée ELF64 :
   ```bash
   gcc -shared -fPIC -o /opt/rocm/lib/librocprofiler_stub.so rocprofiler_stub.c
   ```

### Étape 4 : Configuration des variables d'environnement (`~/.bashrc`)
Ajouter les lignes suivantes à la fin du fichier `~/.bashrc` de ton utilisateur pour automatiser le chargement à chaque connexion à WSL :
```bash
# ROCm + librocdxg (WSL2 - RX 7800 XT)
export PATH="/opt/rocm/bin:$PATH"
export LD_LIBRARY_PATH="/opt/rocm/lib:/usr/lib/wsl/lib:$LD_LIBRARY_PATH"
export HSA_ENABLE_DXG_DETECTION=1
export HSA_OVERRIDE_GFX_VERSION=11.0.1
```

### Étape 5 : Lancement dans le code Python
Dans chaque script Python utilisant PyTorch, charger le stub au tout début du fichier **avant** d'importer `torch` :
```python
import ctypes
try:
    # Charge le stub dans l'espace global du processus
    ctypes.CDLL('/opt/rocm/lib/librocprofiler_stub.so', mode=ctypes.RTLD_GLOBAL)
except Exception:
    pass

import torch
# Ton code PyTorch s'exécute maintenant sur GPU sans crash
```

---

## 4. Diagnostic & Commandes utiles

* **Vérifier l'état matériel global** :
  ```bash
  rocminfo
  ```
  *(Doit lister deux agents : Agent 1 (ton CPU) et Agent 2 (gfx1101, ta carte graphique RX 7800 XT))*

* **Vérifier la détection PyTorch** (depuis ton venv actif) :
  ```bash
  python3 scripts/check_gpu.py
  ```
  *(Doit afficher `ROCm disponible : True` et identifier le GPU `AMD Radeon RX 7800 XT`)*
