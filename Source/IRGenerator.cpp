/*
  ==============================================================================

    IRGenerator.cpp
    Created: 27 Dec 2025 6:00:22pm
    Author:  sebpi

  ==============================================================================
*/

#include <cmath>
#include "IRGenerator.h"

void IRGenerator::deconvolve(const juce::AudioBuffer<float> &drySweep,
                             const juce::AudioBuffer<float> &recordedSweep,
                             juce::AudioBuffer<float> &resultIR,
                             double sampleRate, float duration, float fStart,
                             float fEnd) {
  const float *sweepData = drySweep.getReadPointer(0);
  const float *recData = recordedSweep.getReadPointer(0);
  int sweepLen = drySweep.getNumSamples();
  int recLen = recordedSweep.getNumSamples();

  if (sweepLen <= 0 || recLen < sweepLen) {
    resultIR.clear();
    return;
  }

  // ===================================================================
  // 1. Création de l'inverse Farina (filtre apparié)
  // ===================================================================
  float L = duration / std::log(fEnd / fStart); // log naturel
  std::vector<float> inverse(sweepLen);

  for (int i = 0; i < sweepLen; ++i) {
    float t = static_cast<float>(i) / static_cast<float>(sampleRate);
    float envelope = std::exp(-t / L);
    inverse[i] = sweepData[sweepLen - 1 - i] * envelope;
  }

  // ===================================================================
  // 2. Convolution via FFT (multiplication dans le domaine fréquentiel)
  // ===================================================================
  int convLen = recLen + sweepLen - 1;
  int fftSize = juce::nextPowerOfTwo(convLen);
  int order =
  static_cast<int>(std::round(std::log2(static_cast<double>(fftSize))));
  juce::dsp::FFT fft(order);

  // Buffers de taille fftSize * 2 (format real-only de JUCE)
  std::vector<float> fftBufferRec(fftSize * 2, 0.0f);
  std::vector<float> fftBufferInv(fftSize * 2, 0.0f);

  // Copie des signaux (zero-padding implicite)
  std::copy(recData, recData + recLen, fftBufferRec.begin());
  std::copy(inverse.data(), inverse.data() + sweepLen, fftBufferInv.begin());

  // Forward FFT
  fft.performRealOnlyForwardTransform(fftBufferRec.data());
  fft.performRealOnlyForwardTransform(fftBufferInv.data());

  // Multiplication complexe (Re + jIm)
  for (int i = 0; i < fftSize; ++i) {
    int idx = i * 2;
    float a = fftBufferRec[idx];     // Re recorded
    float b = fftBufferRec[idx + 1]; // Im recorded
    float c = fftBufferInv[idx];     // Re inverse
    float d = fftBufferInv[idx + 1]; // Im inverse

    fftBufferRec[idx] = a * c - b * d;     // Nouvelle partie réelle
    fftBufferRec[idx + 1] = a * d + b * c; // Nouvelle partie imaginaire
  }

  // Inverse FFT
  fft.performRealOnlyInverseTransform(fftBufferRec.data());

  // Scaling (normalisation FFT)
  float gainScale = 1.0f / static_cast<float>(fftSize);
  juce::FloatVectorOperations::multiply(fftBufferRec.data(), gainScale,
                                        fftSize * 2);

  // ===================================================================
  // 3. Recherche du pic pour alignement à t=0
  // ===================================================================
  int peakIndex = 0;
  float maxAbs = 0.0f;
  for (int i = 0; i < convLen; ++i) {
    float val = std::abs(fftBufferRec[i]); // Après IFFT, données valides aux
                                           // indices 0 à convLen-1
    if (val > maxAbs) {
      maxAbs = val;
      peakIndex = i;
    }
  }

  // ===================================================================
  // 4. Préparation de l'IR finale
  // ===================================================================
  int maxSamples =
      static_cast<int>(sampleRate * 0.5); // 500 ms → parfait pour cab guitare
  int samplesAfterPeak = convLen - peakIndex;
  int finalLength = juce::jmin(maxSamples, samplesAfterPeak);

  if (finalLength <= 0) {
    resultIR.clear();
    return;
  }

  resultIR.setSize(1, finalLength);
  resultIR.copyFrom(0, 0, fftBufferRec.data() + peakIndex, finalLength);

  // ===================================================================
  // 5. Normalisation à -6 dB peak
  // ===================================================================
  float currentPeak = resultIR.getMagnitude(0, 0, finalLength);
  if (currentPeak > 0.0f)
    resultIR.applyGain(0.5011872336f / currentPeak); // ≈ -6 dB

  // ===================================================================
  // 6. Fade-out doux (cosine, 20 ms) pour éviter les clics
  // ===================================================================
  int fadeSamples =
      juce::jmin(finalLength, static_cast<int>(sampleRate * 0.02));
  if (fadeSamples > 0) {
    auto *data = resultIR.getWritePointer(0);
    for (int i = 0; i < fadeSamples; ++i) {
      float phase =
          juce::MathConstants<float>::pi * (float)i / (float)fadeSamples;
      float env = 0.5f * (1.0f + std::cos(phase));
      data[finalLength - fadeSamples + i] *= env;
    }
  }
}