# VSTProfiler — Simulateur d’amplificateurs guitare

[![Epitech](https://img.shields.io/badge/Epitech-Paris-blue)](https://www.epitech.eu/fr/)
[![JUCE](https://img.shields.io/badge/JUCE-7.0.9+-brightgreen)](https://juce.com/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++20](https://img.shields.io/badge/C++-20-blue)](https://en.cppreference.com/w/cpp/20)

> **EIP 2025-2026 — Track Technique**  
> Projet de 5ᵉ année (débuté en 4ᵉ année) réalisé par 4 étudiants d’Epitech Paris

## Description

VSTProfiler est un plug-in audio (format VST3 / AU / Standalone) qui permet de **modéliser et reproduire fidèlement le son d’amplificateurs guitare légendaires** à partir de techniques de capture d’impulsions, de modélisation de circuits et d’apprentissage machine léger.

L’objectif : offrir aux guitaristes un outil gratuit, open-source et hautement personnalisable capable de « copier » le timbre et la dynamique d’amplis réels avec une qualité comparable aux solutions commerciales.

Fonctionnalités prévues / en cours :
- Capture d’impulsions (IR Loader + outils de capture maison)
- Modélisation non-linéaire de lampes (12AX7, EL34, etc.)
- Émulations de haut-parleurs et de baffles
- Oversampling jusqu’à 16× pour zéro aliasing
- Interface moderne et skinnable réalisée avec JUCE
- Support M1/M2/Apple Silicon + Windows + Linux (objectif)

## Équipe

| Nom                  | Rôle principal                  |
|----------------------|----------------------------------|
| [QuentinStoll](https://github.com/QuentinStoll)          | Audio DSP / Architecture        |
| [RychlewL](https://github.com/RychlewL)             | JUCE GUI / UX                   |
| [LaurntZhang](https://github.com/LaurntZhang)              | Modélisation physique / IR      |
| [LeCentenaire](https://github.com/LeCentenaire)              | Tests, CI/CD, documentation     |

## Technologies

- **JUCE 7.9+** (framework audio & GUI)
- C++20
- GitHub Actions pour CI

## Pré-requis

- **Projucer** (inclus avec JUCE) → [Télécharger JUCE](https://juce.com/get-juce)
- **Visual Studio 2022** (Community ou supérieur) avec :
  - Charge de travail **Développement Desktop en C++**
  - Support **v143** (ou supérieur)
- **Git** (pour cloner + submodules)
- (Optionnel) **Reaper** (Celui qu'on utilise)

## Collaboration — Comment contribuer efficacement

Vous voulez aider ? Super ! Voici les fichiers clés à modifier selon votre contribution :

| Type de contribution | Fichiers à modifier | Outils |
|----------------------|---------------------|--------|
| **DSP / Audio** (nouveaux modèles d’amplis, oversampling, IR, etc.) | `Source/PluginProcessor.cpp`, `Source/DSP/*.cpp/h` | Visual Studio, Projucer |
| **Interface graphique** (nouveaux knobs, skins, layout) | `Source/PluginEditor.cpp`, `Resources/*.png`, `Source/GUI/*.cpp` | Projucer (GUI Editor), Visual Studio |
| **Nouvelles émulations** (Marshall, Fender, etc.) | Ajouter dans `Source/DSP/Amps/`, référencer dans `PluginProcessor` | Visual Studio |
| **Tests unitaires** | `Tests/` (GoogleTest) | Visual Studio |
| **Documentation / README** | `README.md`, `docs/` | Markdown |
| **Scripts Python** (capture IR, entraînement NN) | `Scripts/` | Python 3.10+ |

### Étapes pour contribuer

1. **Ouvrez le projet avec le Projucer**  
   → `Profiler.jucer` (fichier central du projet)

2. **Ne modifiez JAMAIS le `.jucer` sans accord d’équipe**  
   → Il génère les projets VS, Xcode, etc. → **risque de conflits**

3. **Travaillez dans Visual Studio**  
   → Ouvrez la solution générée : `Builds/VisualStudio2022/Profiler.sln`

4. **Ajoutez vos fichiers via le Projucer**  
   → Cliquez droit → *Add New File* → sauvegardez → commit

5. **Testez en Standalone avant VST3**  
   → Plus rapide à lancer

6. **Commit clair** (exemple) :
   ```bash
   git commit -m "feat: ajout modèle Fender Twin Reverb avec 12AX7"
