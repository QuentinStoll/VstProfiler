#include "DynamicWaveshaper.h"

void AmpProcessor::prepare(float sr) {
    sampleRate = sr;
    attackCoef = std::exp(-1.0f / (0.001f * 1.0f * sr));   // 1ms
    releaseCoef = std::exp(-1.0f / (0.001f * 80.0f * sr)); // 80ms
    fillAsymmetricLUT();
}

void AmpProcessor::fillAsymmetricLUT() {
    for (int i = 0; i < lutSize; ++i) {
        // x va de -1.0 à 1.0
        float x = (2.0f * i / (float)(lutSize - 1)) - 1.0f;

        // CLONAGE : On crée une saturation asymétrique
        if (x >= 0) {
            lut[i] = std::tanh(x * 1.5f); // Cycle positif : saturation franche
        }
        else {
            lut[i] = std::tanh(x * 0.7f) * 0.9f; // Cycle négatif : plus doux, typique lampe
        }
    }
}

float AmpProcessor::readLUT(float input) {
    // 1. On "clamp" pour éviter de sortir de l'index du tableau
    float val = std::clamp(input, -1.0f, 1.0f);

    // 2. Conversion de l'entrée (-1 à 1) vers l'index (0 à 2047)
    float indexPos = (val + 1.0f) * 0.5f * (float)(lutSize - 1);
    int i = (int)indexPos;
    float fraction = indexPos - (float)i;

    // 3. Interpolation linéaire (essentiel pour éviter le bruit)
    if (i >= lutSize - 1) return lut[lutSize - 1];
    return lut[i] + fraction * (lut[i + 1] - lut[i]);
}

float AmpProcessor::processSample(float x) {
    float absx = std::abs(x);
    // Enveloppe (déjà correcte dans ton code)
    if (absx > env) env = attackCoef * env + (1.0f - attackCoef) * absx;
    else env = releaseCoef * env + (1.0f - releaseCoef) * absx;

    // Application du drive dynamique
    float drivenSignal = x * (A + B * env);

    // Sortie via LUT au lieu de tanh direct
    return readLUT(drivenSignal);
}