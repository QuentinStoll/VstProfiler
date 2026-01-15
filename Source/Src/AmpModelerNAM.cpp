//#include "AmpModelerNAM.h"
//
//bool AmpModelerNAM::loadModel(const juce::File &modelFile) {
//  if (!modelFile.existsAsFile())
//    return false;
//
//  std::ifstream file(modelFile.getFullPathName().toStdString());
//  if (!file.is_open())
//    return false;
//
//  nlohmann::json json;
//  try {
//    file >> json;
//  } catch (...) {
//    return false;
//  }
//
//  model = nam::Model::from_json(json);
//  if (!model)
//    return false;
//
//  inputGain = model->GetInputGain();
//  outputGain = model->GetOutputGain();
//
//  DBG("NAM model loaded: " << modelFile.getFileName());
//  DBG("  Input gain: " << inputGain << " dB");
//  DBG("  Output gain: " << outputGain << " dB");
//
//  return true;
//}
//
//void AmpModelerNAM::prepareToPlay(double sampleRate, int samplesPerBlock) {
//  if (model)
//    model->SetSampleRate(static_cast<float>(sampleRate));
//}
//
//void AmpModelerNAM::processBlock(juce::AudioBuffer<float> &buffer) {
//  if (!model || buffer.getNumChannels() == 0)
//    return;
//
//  const int numSamples = buffer.getNumSamples();
//  auto *channelData = buffer.getWritePointer(0);
//
//  juce::FloatVectorOperations::multiply(
//      channelData, juce::Decibels::decibelsToGain(inputGain), numSamples);
//
//  for (int i = 0; i < numSamples; ++i) {
//    float in = channelData[i];
//    float out = model->Process(in);
//    channelData[i] = out;
//  }
//
//  juce::FloatVectorOperations::multiply(
//      channelData, juce::Decibels::decibelsToGain(outputGain), numSamples);
//
//  for (int i = 0; i < numSamples; ++i) {
//    float &s = channelData[i];
//    if (std::abs(s) > 1.0f)
//      s = s > 0 ? 1.0f - std::exp(-(s - 1.0f)) : -1.0f + std::exp((s + 1.0f));
//  }
//}
//
//void AmpModelerNAM::reset() {
//  if (model)
//    model->Reset();
//}