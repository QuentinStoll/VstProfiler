#include "AmpEngine.h"

/**
 * Initializes the processor with the current sample rate.
 * Sets up the smoothing coefficients for the envelope follower and populates the LUT.
 * * @param sr The project sample rate (e.g., 44100.0 or 48000.0).
 */
void AmpProcessor::prepare(float sr) {
    sampleRate = sr;
    
    // Attack time: ~1ms. Converts time constant to recursive filter coefficient.
    attackCoef = std::exp(-1.0f / (0.001f * 1.0f * sr));
    
    // Release time: ~80ms. Provides a natural decay for the dynamic drive.
    releaseCoef = std::exp(-1.0f / (0.001f * 80.0f * sr));
    
    fillAsymmetricLUT();
}

/**
 * Populates the Look-Up Table (LUT) with an asymmetric saturation curve.
 * This simulates the behavior of tube stages where positive and negative 
 * cycles are clipped differently, creating even-order harmonics.
 */
void AmpProcessor::fillAsymmetricLUT() {
    for (int i = 0; i < lutSize; ++i) {
        // Map index 'i' to a range of [-1.0, 1.0]
        float x = (2.0f * i / (float)(lutSize - 1)) - 1.0f;

        // Positive cycle: Harder saturation (shaping the "crunch")
        if (x >= 0) {
            lut[i] = std::tanh(x * 1.5f);
        }
        // Negative cycle: Softer saturation (shaping the "warmth")
        else {
            lut[i] = std::tanh(x * 0.7f) * 0.9f;
        }
    }
}

/**
 * Performs a high-performance lookup in the pre-calculated saturation table.
 * Uses linear interpolation to prevent aliasing and quantization noise.
 * * @param input The drive-multiplied signal to be saturated.
 * @return The saturated output sample.
 */
float AmpProcessor::readLUT(float input) {
    // 1. Clamp input to ensure it stays within the LUT's defined range [-1.0, 1.0]
    float val = std::clamp(input, -1.0f, 1.0f);

    // 2. Map the normalized value to the table index range [0, lutSize - 1]
    float indexPos = (val + 1.0f) * 0.5f * (float)(lutSize - 1);
    int i = (int)indexPos;
    float fraction = indexPos - (float)i;

    // 3. Linear interpolation between index 'i' and 'i+1'
    if (i >= lutSize - 1) return lut[lutSize - 1];
    
    return lut[i] + fraction * (lut[i + 1] - i[lut]);
}

/**
 * Processes a single audio sample through the dynamic amp simulation.
 * Includes an envelope follower to modulate the drive amount based on input dynamics.
 * * @param x The raw input audio sample.
 * @return The processed (distorted) output sample.
 */
float AmpProcessor::processSample(float x) {
    // 1. Enveloppe (inchangée)
    float absx = std::abs(x);
    if (absx > env) {
        env = attackCoef * env + (1.0f - attackCoef) * absx;
    } else {
        env = releaseCoef * env + (1.0f - releaseCoef) * absx;
    }

    // 2. SATURATION FIXE
    // On multiplie par 2.0 (ou 4.0) juste pour être sûr que ça sature un peu
    float saturated = readLUT(x * (2.0f + B * env));

    // 3. GAIN STATIQUE (A)
    // On applique A à la fin. C'est ce qu'on veut tester.
    return saturated * A; 
}