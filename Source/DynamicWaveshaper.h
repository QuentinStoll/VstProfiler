#pragma once

#include <cmath>

struct DynamicWaveshaper
{
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

    void prepare(float sr)
    {
        sampleRate = sr;
        attackCoef = std::exp(-1.0f / (0.001f * attackMs * sr));
        releaseCoef = std::exp(-1.0f / (0.001f * releaseMs * sr));
        env = 0.0f;
    }

    inline float processSample(float x)
    {
        float absx = std::abs(x);

        // enveloppe avec attaque rapide / release lent
        if (absx > env)
            env = attackCoef * env + (1.0f - attackCoef) * absx;
        else
            env = releaseCoef * env + (1.0f - releaseCoef) * absx;

        float drive = A + B * env;

        // tube sat sim
        return std::tanh(drive * x);
    }
};
