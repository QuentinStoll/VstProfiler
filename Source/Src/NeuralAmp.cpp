#include "NeuralAmp.h"

NeuralAmp::NeuralAmp()
{
}

void NeuralAmp::prepare(double sampleRate, int samplesPerBlock)
{
    reset();
}

void NeuralAmp::reset()
{
    model.reset();
}

bool NeuralAmp::loadModel(const juce::File& modelFile)
{
    if (!modelFile.existsAsFile()) return false;

    try {
        auto modelText = modelFile.loadFileAsString().toStdString();
        auto json = nlohmann::json::parse(modelText);
        model.parseJson(json);
        model.reset();
        return true;
    } 
    catch (...) {
        return false;
    }
}

void NeuralAmp::processBlock(float* buffer, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        buffer[i] = model.forward(&buffer[i]);
    }
}