#include "Modules/MasterSlidersModule.h"

MasterSlidersModule::MasterSlidersModule(juce::AudioProcessorValueTreeState& apvts) {
    addAndMakeVisible(_masterVolumeKnob);
    addAndMakeVisible(_gainKnob);
    addAndMakeVisible(_noiseGateKnob);

    _masterVolumeAttachment = std::make_unique<SliderAttachment>(apvts, "master", _masterVolumeKnob.getSlider());
    _gainAttachment = std::make_unique<SliderAttachment>(apvts, "gain", _gainKnob.getSlider());
    _noiseGateAttachment = std::make_unique<SliderAttachment>(apvts, "noise", _noiseGateKnob.getSlider());
}

MasterSlidersModule::~MasterSlidersModule() {}

void MasterSlidersModule::paint(juce::Graphics& /*g*/) {}

void MasterSlidersModule::resized() {
    auto area = getLocalBounds();
    auto areaWidth = area.getWidth();

    auto sideArea = area.removeFromLeft(areaWidth / 3);
    auto noiseGateArea = sideArea.removeFromTop(static_cast<int>(getHeight() * 0.5f));
    auto gainArea = sideArea;

    _masterVolumeKnob.setBounds(area);
    _noiseGateKnob.setBounds(noiseGateArea);
    _gainKnob.setBounds(gainArea);
}