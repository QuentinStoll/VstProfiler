#include "UsingScreen.h"

// This class is for the Use mode of the plugin
UsingScreen::UsingScreen(juce::AudioProcessorValueTreeState& apvts) {
    _mainSettingsSection = std::make_unique<MainSettingsSection>(apvts);
    _eqNormalizeSection = std::make_unique<EqNormalizeSection>();
    _ampLoaderSection = std::make_unique<AmpLoaderSection>();

    addAndMakeVisible(_mainSettingsSection.get());
    addAndMakeVisible(_eqNormalizeSection.get());
    addAndMakeVisible(_ampLoaderSection.get());

    setSize(600, 400);
}

UsingScreen::~UsingScreen() {}

void UsingScreen::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::darkblue);
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(20.0f));
    g.drawFittedText("Entering Using mode", getLocalBounds(), juce::Justification::centredTop, 1);
}

void UsingScreen::resized() {
    auto bounds = getLocalBounds();

    bounds.removeFromTop(50);

    auto mainSettingsArea = bounds.removeFromTop((int)(bounds.getHeight() * 0.4));
    _mainSettingsSection->setBounds(mainSettingsArea);

    auto eqNormalizeArea = bounds.removeFromTop(65);
    _eqNormalizeSection->setBounds(eqNormalizeArea);

    /*auto ampLoaderArea = bounds.removeFromTop(50);
    _ampLoaderSection->setBounds(ampLoaderArea);*/
}