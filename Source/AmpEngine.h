#pragma once

#include <JuceHeader.h>
#include <cmath>

class AmpProcessor : public juce::Component
{
public:
    void prepare(float sr);

    float processSample(float x);

private:
    // --- LUT ---
    static const int lutSize = 2048;
    float lut[lutSize];
    void fillAsymmetricLUT();
    float readLUT(float input);

    // --- Dynamic ---
    float env = 0.0f;
    float sampleRate = 44100.0f;
    float A = 4.0f;   // Gain de base
    float B = 10.0f;  // Réaction à l'enveloppe
    float attackCoef, releaseCoef;
};
