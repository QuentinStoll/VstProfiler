#include "Modules/BasicEqModule.h"

BasicEqModule::BasicEqModule(juce::AudioProcessorValueTreeState& apvts) {
    addAndMakeVisible(_bassKnob);
    addAndMakeVisible(_midKnob);
    addAndMakeVisible(_trebleKnob);
    addAndMakeVisible(_presenceKnob);
    addAndMakeVisible(_depthKnob);

    _bassAttachment = std::make_unique<SliderAttachment>(apvts, "bass", _bassKnob.getSlider());
    _midAttachment = std::make_unique<SliderAttachment>(apvts, "mid", _midKnob.getSlider());
    _trebleAttachment = std::make_unique<SliderAttachment>(apvts, "treble", _trebleKnob.getSlider());
    _presenceAttachment = std::make_unique<SliderAttachment>(apvts, "presence", _presenceKnob.getSlider());
    _depthAttachment = std::make_unique<SliderAttachment>(apvts, "depth", _depthKnob.getSlider());
}

BasicEqModule::~BasicEqModule() {
}

void BasicEqModule::paint(juce::Graphics& g) {
}

void BasicEqModule::resized() {
    auto area = getLocalBounds();
    auto areaWidth = area.getWidth();

    auto topArea = area.removeFromTop(getHeight() * 0.5f);

    auto bassArea = topArea.removeFromLeft(areaWidth / 3);
    auto midArea = topArea.removeFromLeft(areaWidth / 3);
    auto trebleArea = topArea;

    area = area.withSizeKeepingCentre(bassArea.getWidth() * 2, area.getHeight());

    auto presenceArea = area.removeFromLeft(bassArea.getWidth());
    auto depthArea = area.removeFromLeft(bassArea.getWidth());

    _bassKnob.setBounds(bassArea);
    _midKnob.setBounds(midArea);
    _trebleKnob.setBounds(trebleArea);
    _presenceKnob.setBounds(presenceArea);
    _depthKnob.setBounds(depthArea);
}