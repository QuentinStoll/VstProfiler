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
- Support Windows (M1/M2/Apple Silicon + Linux (objectif))

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

- CMake 3.22 ou supérieur
- Compilateur supportant C++20 (MSVC 2022, clang 14+, GCC 11+)
- JUCE 7 (téléchargeable via le Projucer ou en submodule)

## Build

```bash
git clone --recursive https://github.com/QuentinStoll/Profiler.git
cd Profiler
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release --target Profiler_Standalone Profiler_VST3
