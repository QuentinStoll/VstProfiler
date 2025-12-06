/*
  ==============================================================================

    MainSettingsSection.h
    Created: 1 Dec 2025 11:40:09pm
    Author:  Laurent ZHANG

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "RotarySlider.h"

class MainSettingsSection : public juce::Component
{
public:
	MainSettingsSection();
	~MainSettingsSection();

	void paint(juce::Graphics& g) override;
	void resized() override;

private:
	std::vector<std::unique_ptr<RotarySlider>> _sliders;
};