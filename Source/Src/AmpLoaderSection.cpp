#include "AmpLoaderSection.h"

AmpLoaderSection::AmpLoaderSection() {}

AmpLoaderSection::~AmpLoaderSection() {}

void AmpLoaderSection::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::aqua);
    g.setColour(juce::Colours::green);
    g.drawRect(getLocalBounds().toFloat(), 1.0f);
}

void AmpLoaderSection::resized() {
    auto bounds = getLocalBounds();
}