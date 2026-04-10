#pragma once

#include <JuceHeader.h>

#include <cmath>

/**
 * @class AmpProcessor
 * @brief A lightweight asymmetric amplifier simulation with dynamic drive.
 * * This class implements a tube-style saturation using a pre-computed
 * Look-Up Table (LUT) and an envelope follower to modulate the gain
 * based on the input signal's dynamics.
 */
class AmpProcessor : public juce::Component {
   public:
    void prepare(float sr);
    float processSample(float x);

    void setA(float newA) { A = newA; }
    float getA() const { return A; }

    int getLutSize() { return lutSize; };

    void updateLUT(const std::vector<float>& newValues) {
        if (newValues.size() != (size_t)lutSize)
            return;

        for (int i = 0; i < lutSize; ++i) {
            lut[i] = newValues[i];
        }

        //                              IMPORTANT
        // Optionnel : Normalisation
        // Si l'enregistrement WET était trop faible, on peut normaliser la LUT
        // pour qu'elle atteigne 1.0 à l'index max.
    };

   private:
    // --- Look-Up Table (LUT) Management ---

    /** Size of the saturation table. Higher values reduce interpolation noise. */
    static const int lutSize = 2048;

    /** The pre-computed saturation curve. */
    float lut[lutSize];

    void fillAsymmetricLUT();
    float readLUT(float input);

    // --- Dynamic Drive & Envelope Following ---

    /** Current state of the envelope follower. */
    float env = 0.0f;

    /** Project sample rate, used for coefficient calculations. */
    float sampleRate = 44100.0f;

    /** Base Gain: Static drive amount applied to the signal. */
    float A = 1.0f;

    /** Dynamic Sensitivity: Determines how much the envelope affects the drive. */
    float B = 5.0f;

    /** Time constants for the envelope follower's response. */
    float attackCoef, releaseCoef;
};