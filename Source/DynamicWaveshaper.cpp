#include "DynamicWaveshaper.h"

void AmpProcessor::prepare(float sr)
{
    sampleRate = sr;
    attackCoef = std::exp(-1.0f / (0.001f * attackMs * sr));
    releaseCoef = std::exp(-1.0f / (0.001f * releaseMs * sr));
    env = 0.0f;
}

float AmpProcessor::processSample(float x)
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
