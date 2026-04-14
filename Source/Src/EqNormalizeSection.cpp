#include "EqNormalizeSection.h"

EqNormalizeSection::EqNormalizeSection() {}

EqNormalizeSection::~EqNormalizeSection() {}

void EqNormalizeSection::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::beige);
    g.setColour(juce::Colours::brown);
    g.drawRect(getLocalBounds().toFloat(), 1.0f);
}

void EqNormalizeSection::resized() { auto bounds = getLocalBounds(); }