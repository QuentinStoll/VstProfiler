/*
  ==============================================================================

    SelectionScreen.cpp
    Created: 30 Nov 2025 2:10:52pm
    Author:  Laurent ZHANG

  ==============================================================================
*/

#include "SelectionScreen.h"
#include <cstdio>

SelectionScreen::SelectionScreen(ScreenSelectedCallback callback) : screenSelectedCallback(callback)
{
	addAndMakeVisible(cloneButton);
	addAndMakeVisible(useButton);

    cloneButton.onClick = [this] {
		printf("Clone button clicked\n");
        screenSelectedCallback(ScreenID::Cloning);
	};

    useButton.onClick = [this] {
        screenSelectedCallback(ScreenID::Using);
	};

    setSize(600, 400);
}

SelectionScreen::~SelectionScreen()
{
}

void SelectionScreen::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::lightgrey);
}

void SelectionScreen::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

    auto bounds = getLocalBounds();
    auto buttonWidth = bounds.getWidth() / 2 - 20;
    auto buttonHeight = 40;

    cloneButton.setBounds(10, bounds.getCentreY() - 20, buttonWidth, buttonHeight);
    useButton.setBounds(bounds.getCentreX() + 10, bounds.getCentreY() - 20, buttonWidth, buttonHeight);
}