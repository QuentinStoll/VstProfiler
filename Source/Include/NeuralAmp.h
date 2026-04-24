#pragma once
#include <RTNeural/RTNeural.h>
#include <juce_audio_basics/juce_audio_basics.h>

class NeuralAmp
{
public:
    NeuralAmp();
    ~NeuralAmp() = default;

    // Architecture : 1 entrée, 1 sortie, 1 couche LSTM de 24 unités, 1 couche Dense
    // C'est le standard pour l'émulation d'amplis à lampes (NAM/AIDA-X style)
    using ModelType = RTNeural::ModelT<float, 1, 1,
        RTNeural::LSTMLayerT<float, 1, 24>,
        RTNeural::DenseT<float, 24, 1>>;

    void prepare(double sampleRate, int samplesPerBlock);
    void reset();
    
    // Charge les poids depuis un fichier JSON (généré par l'entraînement Python)
    bool loadModel(const juce::File& modelFile);

    // Traitement d'un échantillon
    forcedinline float processSample(float input)
    {
        return model.forward(&input);
    }

    // Traitement d'un buffer complet (plus performant)
    void processBlock(float* buffer, int numSamples);

private:
    ModelType model;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NeuralAmp)
};