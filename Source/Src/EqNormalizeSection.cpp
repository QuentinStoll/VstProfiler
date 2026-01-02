/*
  ==============================================================================

    EqNormalizeSection.cpp
    Created: 2 Dec 2025 3:48:38pm
    Author:  Laurent ZHANG

  ==============================================================================
*/

#include "EqNormalizeSection.h"

EqNormalizeSection::EqNormalizeSection()
{
}

EqNormalizeSection::~EqNormalizeSection()
{
}

void EqNormalizeSection::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::beige);
    g.setColour(juce::Colours::brown);
    g.drawRect(getLocalBounds().toFloat(), 1.0f);
}

void EqNormalizeSection::resized()
{
    auto bounds = getLocalBounds();
}