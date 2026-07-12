#include "Modules/AdvancedEqModule.h"

AdvancedEqModule::AdvancedEqModule(juce::AudioProcessorValueTreeState& apvts) {
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

    addAndMakeVisible(_bandLabelsZone);
}

AdvancedEqModule::~AdvancedEqModule() {
}

void AdvancedEqModule::paint(juce::Graphics& g) {
    auto area = _bandLabelsZone.getBounds().toFloat();
    const float cornerSize = 8.0f;

    g.setColour(juce::Colours::blue.withAlpha(0.2f));
    g.fillRoundedRectangle(area, cornerSize);

    g.setColour(juce::Colours::black.withAlpha(0.4f));
    g.drawRoundedRectangle(area.reduced(0.5f), cornerSize, 1.0f);
}

void AdvancedEqModule::resized() {
    auto area = getLocalBounds();
    auto areaWidth = area.getWidth();

    auto topArea = area.removeFromTop(static_cast<int>(getHeight() * 0.4f));

    auto bassArea = topArea.removeFromLeft(areaWidth / 5);
    auto midArea = topArea.removeFromLeft(areaWidth / 5);
    auto trebleArea = topArea.removeFromLeft(areaWidth / 5);
    auto presenceArea = topArea.removeFromLeft(areaWidth / 5);
    auto depthArea = topArea;

    _bassKnob.setBounds(bassArea);
    _midKnob.setBounds(midArea);
    _trebleKnob.setBounds(trebleArea);
    _presenceKnob.setBounds(presenceArea);
    _depthKnob.setBounds(depthArea);

    _bandLabelsZone.setBounds(area);
}