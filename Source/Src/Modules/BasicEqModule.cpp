#include "Modules/BasicEqModule.h"

BasicEqModule::BasicEqModule(juce::AudioProcessorValueTreeState& apvts) {
    addAndMakeVisible(_bassSlider);
    addAndMakeVisible(_midSlider);
    addAndMakeVisible(_trebleSlider);
    addAndMakeVisible(_presenceSlider);
    addAndMakeVisible(_depthSlider);

    _bassAttachment = std::make_unique<SliderAttachment>(apvts, "bass", _bassSlider.getSlider());
    _midAttachment = std::make_unique<SliderAttachment>(apvts, "mid", _midSlider.getSlider());
    _trebleAttachment = std::make_unique<SliderAttachment>(apvts, "treble", _trebleSlider.getSlider());
    _presenceAttachment = std::make_unique<SliderAttachment>(apvts, "presence", _presenceSlider.getSlider());
    _depthAttachment = std::make_unique<SliderAttachment>(apvts, "depth", _depthSlider.getSlider());
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

    _bassSlider.setBounds(bassArea);
    _midSlider.setBounds(midArea);
    _trebleSlider.setBounds(trebleArea);
    _presenceSlider.setBounds(presenceArea);
    _depthSlider.setBounds(depthArea);
}