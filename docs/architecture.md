# Architecture de GeneSynth

## 1. Vue d'Ensemble
GeneSynth est un synthétiseur virtuel (plugin VST3/AU/Standalone) développé en C++ et basé sur le framework JUCE.
L'architecture a été pensée de manière modulaire, orientée objet (OOP) et hiérarchique, afin de séparer strictement :
- **Le traitement du signal (DSP)**
- **La gestion de l'état (Paramètres APVTS)**
- **L'interface graphique (UI)**

## 2. Le Moteur Audio et les Blocs (`aBlock`)
Le cœur du traitement audio repose sur l'interface de base `aBlock`. Toutes les entités qui traitent le son héritent de cette classe, qui impose le cycle de vie classique de JUCE DSP :
- `prepare(const juce::dsp::ProcessSpec& spec)` : Appelé avant la lecture pour initialiser les buffers et taux d'échantillonnage.
- `reset()` : Appelé pour réinitialiser l'état interne (ex: vider les buffers d'un filtre ou réinitialiser la phase).
- `process(const juce::dsp::ProcessContextReplacing<float>& context)` : Appelé pour traiter un bloc audio (ou un échantillon).

### Les Conteneurs (`aContainer`)
Pour organiser les `aBlock`, deux conteneurs principaux sont utilisés :
- **`SeriesContainer`** : Traite ses sous-blocs de manière séquentielle (le signal sortant de A entre dans B).
- **`ParallelContainer`** : Traite ses sous-blocs de manière isolée en parallèle, puis somme (mixe) leurs sorties respectives.

## 3. Hiérarchie Globale
Le point d'entrée audio du plugin (`PluginProcessor::processBlock`) délègue tout le travail à un unique objet **`SynthEngine`** (qui est un `SeriesContainer`).

Le `SynthEngine` exécute en série :
1. **`PolyphonyManager`** : Un wrapper personnalisé autour de `juce::Synthesiser`. Il gère le déclenchement des notes MIDI, le vol de voix, et additionne la sortie audio de toutes les voix actives.
2. **`ReverbModule`** : Un effet global (héritant de `aBlock`) appliqué à la somme des voix polyphoniques, gérant la taille, la longueur et le mix de la réverbération.

## 4. Architecture d'une Voix Polyphonique (`SynthVoice`)
Chaque note jouée déclenche une instance de `SynthVoice` (héritant de `juce::SynthesiserVoice`).
Au sein d'une voix, le calcul audio s'effectue **échantillon par échantillon** (Audio-Rate) pour garantir une modulation parfaite et éviter les bruits de blocs.

Le signal traverse successivement :
1. **`ExcitationSection`** : Génère la source sonore. Elle contient :
   - `OscB` : Évalué en premier, son signal brut sert de modulateur (FM).
   - `OscA` : Onde principale dont la fréquence peut être modulée à l'Audio-Rate par le signal de `OscB`.
   - `NoiseGenerator` : Génère du bruit blanc.
   La sortie finale de l'excitation est la somme pondérée de l'OscA, de l'OscB (si son volume est > 0) et du Bruit.
2. **`FilterSection`** : Applique un filtre (ex: `LadderFilter`) pour sculpter le contenu harmonique du son généré par l'excitation.
3. **Amplification finale** : Le volume global de la voix est multiplié par l'enveloppe ADSR globale de la note pour gérer l'attaque et le relâchement final.

## 5. Le Système de Modulation (Macro Signal)
Chaque voix possède son propre générateur de modulation composite appelé **MacroSignal**. Ce signal varie entre -1.0 et 1.0 et est calculé à partir de :
- L'enveloppe de modulation (`ENV_MOD` - ADSR)
- Le LFO (Oscillateur Basse Fréquence)
- L'enveloppe globale (`ENV_GLOBAL` - ADSR)

À chaque échantillon audio, la voix calcule la valeur instantanée du MacroSignal et la transmet par pointeur à tous les oscillateurs et filtres.

## 6. Le Système de Paramètres (OOP)
Pour éviter que chaque petit composant doive interroger le gros arbre d'état JUCE (APVTS) à chaque échantillon, un système de paramètres orienté objet a été mis en place.
- **`StaticParameter`** : Pointeur vers un paramètre simple (ex: Choix de la forme d'onde, Temps de Reverb). Il est lu directement.
- **`ModulableParameter`** : Paramètre complexe composé d'une "Valeur de base" et d'une "Quantité de modulation" (`Mod Amount`). 

Les composants du moteur (comme `OscA` ou `LadderFilter`) n'ont accès qu'à ces pointeurs de paramètres, et appellent `getRawValue()` ou `getFinalValue(macro)` pour obtenir la valeur réelle à utiliser. Le câblage (injection des dépendances) est effectué tout au début par le `PluginProcessor`.

## 7. L'Interface Utilisateur (UI)
L'interface graphique est dessinée par `GeneSynthAudioProcessorEditor`, mais elle est modulaire :
- Elle est décomposée en plusieurs sous-sections indépendantes (`OscASection`, `FilterSection`, `GlobalEnvSection`, etc.).
- Toutes ces sections héritent de `BaseSection`.
- Chaque section s'occupe exclusivement de créer, placer et lier ses propres contrôles visuels (potards/boutons) à l'APVTS.
- Cette séparation permet de concevoir rapidement l'UI et de déplacer des blocs de potards sans casser le reste de l'interface.

## 8. Intelligence Artificielle et Inférence
L'intégration de l'IA (transfert de timbre et génération de patch) repose sur les principes suivants :

- **Normalisation du Pitch (C5)** : Lorsqu'un sample est importé, son pitch fondamental est détecté (via l'algorithme **MPM**). Le sample est ensuite repitché (normalisé) sur un Do 5 (C5) avant d'être analysé. Cela permet au réseau de neurones de se concentrer exclusivement sur le timbre (paramètres du synthé) sans être perturbé par la hauteur de la note.
- **Décodeur Latent et Morph Pad** : L'espace latent du réseau de neurones (les gènes abstraits du son) est exposé dans l'APVTS via des paramètres invisibles. Le **Morph Pad** de l'UI permet de naviguer dans cet espace latent. À chaque mouvement, le réseau de neurones (couche de décodage) recalcule et met à jour instantanément les paramètres classiques du synthé (Osc, Filtre, Enveloppes).
- **Modèles Dynamiques** : Le moteur d'inférence permet de charger dynamiquement des modèles d'IA externes (`.onnx` ou `.json`). L'utilisateur n'est pas bloqué sur une seule IA et peut importer des modèles entraînés sur des spécificités différentes (ex: modèle spécialisé FM, modèle spécialisé Analogique).
## 8. Intelligence Artificielle et Inférence
L'intégration de l'IA (transfert de timbre et génération de patch) repose sur les principes suivants :

- **Normalisation du Pitch (C5)** : Lorsqu'un sample est importé, son pitch fondamental est détecté (via l'algorithme **MPM**). Le sample est ensuite repitché (normalisé) sur un Do 5 (C5) avant d'être analysé. Cela permet au réseau de neurones de se concentrer exclusivement sur le timbre (paramètres du synthé) sans être perturbé par la hauteur de la note.
- **Décodeur Latent et Morph Pad** : L'espace latent du réseau de neurones (les gènes abstraits du son) est exposé dans l'APVTS via des paramètres invisibles. Le **Morph Pad** de l'UI permet de naviguer dans cet espace latent. À chaque mouvement, le réseau de neurones (couche de décodage) recalcule et met à jour instantanément les paramètres classiques du synthé (Osc, Filtre, Enveloppes).
- **Modèles Dynamiques** : Le moteur d'inférence (basé préférentiellement sur **ONNX Runtime**) permet de charger dynamiquement des modèles d'IA externes (). L'utilisateur n'est pas bloqué sur une seule IA et peut importer des modèles entraînés sur des spécificités différentes (ex: modèle spécialisé FM, modèle spécialisé Analogique).
