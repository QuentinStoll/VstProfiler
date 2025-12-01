/*
  ==============================================================================

    UsingScreen.cpp
    Created: 30 Nov 2025 2:06:43pm
    Author:  Laurent ZHANG

  ==============================================================================
*/

#include "UsingScreen.h"

UsingScreen::UsingScreen()
{
	setSize(600, 400);
}

UsingScreen::~UsingScreen()
{
}

void UsingScreen::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkblue);
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(20.0f));
    g.drawFittedText("Entering Using mode", getLocalBounds(), juce::Justification::centred, 1);
}

void UsingScreen::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
}