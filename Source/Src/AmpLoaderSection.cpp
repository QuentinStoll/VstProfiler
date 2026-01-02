/*
  ==============================================================================

    AmpLoaderSection.cpp
    Created: 2 Dec 2025 3:49:03pm
    Author:  Laurent ZHANG

  ==============================================================================
*/

#include "AmpLoaderSection.h"

AmpLoaderSection::AmpLoaderSection()
{
}

AmpLoaderSection::~AmpLoaderSection()
{
}

void AmpLoaderSection::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::aqua);
    g.setColour(juce::Colours::green);
    g.drawRect(getLocalBounds().toFloat(), 1.0f);
}

void AmpLoaderSection::resized()
{
    auto bounds = getLocalBounds();
}