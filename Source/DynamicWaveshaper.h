#pragma once

#include <JuceHeader.h>
#include <cmath>

class AmpProcessor : public juce::Component
{
public:
    void prepare(float sr);

    float processSample(float x);

private:
    float env = 0.0f;
    float sampleRate = 44100.0f;

    // paramètres "profil"
    float A = 6.0f;   // Default drive
    float B = 15.0f;   // Drive augmentation with level

    // time const
    float attackMs = 1.0f;
    float releaseMs = 80.0f;

    float attackCoef = 0.0f;
    float releaseCoef = 0.0f;
};
