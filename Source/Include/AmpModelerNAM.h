//#pragma once
//
//#include "NeuralAmpModelerCore/nam.h" 
//#include "nlohmann/json.hpp"
//#include <JuceHeader.h>
//
//class AmpModelerNAM {
//public:
//  AmpModelerNAM() = default;
//  ~AmpModelerNAM() { reset(); }
//
//  bool loadModel(const juce::File &modelFile);
//
//  void prepareToPlay(double sampleRate, int samplesPerBlock);
//
//  void processBlock(juce::AudioBuffer<float> &buffer);
//
//  void reset();
//
//  bool isModelLoaded() const { return model != nullptr; }
//
//private:
//  std::unique_ptr<nam::Model> model;
//  float inputGain = 0.0f;
//  float outputGain = 0.0f;
//};