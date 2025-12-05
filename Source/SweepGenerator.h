#pragma once
#include <cmath>
#include <corecrt_math_defines.h>

class SweepGenerator
{
public:
    SweepGenerator() = default;

    static void generateLogSweep(juce::AudioBuffer<float>& buffer,
        double sampleRate,
        float durationSeconds = 15.0f,
        float fStart = 20.0f,
        float fEnd = 20000.0f,
        float amplitude = 0.5f)
    {
        int numSamples = int(sampleRate * durationSeconds);
        buffer.setSize(1, numSamples); // mono
        float* writePtr = buffer.getWritePointer(0);

        const double K = durationSeconds / std::log(fEnd / fStart);

        // Paramètres du fade (en secondes)
        float fadeTime = 0.05f; // 50 ms
        int fadeSamples = int(fadeTime * sampleRate);

        for (int n = 0; n < numSamples; ++n)
        {
            double t = n / sampleRate;
            double freq = fStart * std::exp(t / K);
            float sample = amplitude * std::sin(juce::MathConstants<double>::twoPi * freq * t);

            // Fade-in
            if (n < fadeSamples)
                sample *= float(n) / float(fadeSamples);

            // Fade-out
            if (n > numSamples - fadeSamples)
                sample *= float(numSamples - n) / float(fadeSamples);

            writePtr[n] = sample;
        }
    }
};
