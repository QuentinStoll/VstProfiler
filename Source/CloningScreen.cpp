/*
  ==============================================================================

    CloningScreen.cpp
    Created: 30 Nov 2025 1:39:18pm
    Author:  Laurent ZHANG

  ==============================================================================
*/

#include "CloningScreen.h"

CloningScreen::CloningScreen()
{
	setSize(600, 400);
}

CloningScreen::~CloningScreen()
{
}

void CloningScreen::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(20.0f));
	g.drawFittedText("Entering Cloning mode", getLocalBounds(), juce::Justification::centred, 1);
}

void CloningScreen::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
}