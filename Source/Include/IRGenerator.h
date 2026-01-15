/*
  ==============================================================================

    IRGenerator.h
    Created: 27 Dec 2025 6:01:16pm
    Author:  sebpi

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <complex>

class IRGenerator : public juce::Component {
public:
  /**
   * Déconvolution d'un logarithmic sine sweep (méthode Farina) pour générer une
   * Impulse Response.
   *
   * @param drySweep        Le sweep original (signal envoyé au cabinet)
   * @param recordedSweep   Le sweep enregistré au micro
   * @param resultIR        Buffer qui recevra l'IR finale (mono)
   * @param sampleRate      Taux d'échantillonnage des deux signaux
   * @param duration        Durée du sweep en secondes (ex: 15.0f)
   * @param fStart          Fréquence de départ du sweep (ex: 20.0 Hz)
   * @param fEnd            Fréquence de fin du sweep (ex: 20000.0 Hz)
   */
  static void deconvolve(const juce::AudioBuffer<float> &drySweep,
                         const juce::AudioBuffer<float> &recordedSweep,
                         juce::AudioBuffer<float> &resultIR, double sampleRate,
                         float duration = 15.0f, float fStart = 20.0f,
                         float fEnd = 20000.0f);

private:
  IRGenerator() = delete;
};