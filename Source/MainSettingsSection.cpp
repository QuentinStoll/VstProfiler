/*
  ==============================================================================

    MainSettingsSection.cpp
    Created: 1 Dec 2025 11:39:55pm
    Author:  Laurent ZHANG

  ==============================================================================
*/

#include "MainSettingsSection.h"

MainSettingsSection::MainSettingsSection()
{
	// Define the parameters for each rotary slider
	std::vector<RotarySliderParameter> sliderParams = {
		{ "Input", " dB", -12.0f, 12.0f, 0.0f },
		{ "Gate" , " dB", 0.0f, 10.0f, 0.0f },
		{ "Bass", "", -15.0f, 15.0f, 0.0f },
		{ "Mid", "", -15.0f, 15.0f, 0.0f },
		{ "Treble", "", -15.0f, 15.0f, 0.0f },
		{ "Output", " dB", -48.0f, 12.0f, -6.0f }
	};

	// Create and add the rotary sliders based on the defined parameters
	for (const auto& param : sliderParams)
	{
		auto slider = std::make_unique<RotarySlider>(param);
		_sliders.push_back(std::move(slider));
		addAndMakeVisible(_sliders.back().get());
	}
}

MainSettingsSection::~MainSettingsSection()
{
}

void MainSettingsSection::paint(juce::Graphics& g)
{
	g.fillAll(juce::Colours::darkgrey);
	g.setColour(juce::Colours::darkgrey);
	g.drawRect(getLocalBounds().toFloat(), 1.0f);
}

void MainSettingsSection::resized()
{
	// Define the area for the sliders with some padding
	auto area = getLocalBounds().reduced(8);
	const int count = (int)_sliders.size();
	if (count == 0) return;

	// Calculate width and height for each slider
	const int w = area.getWidth() / count;
	const int h = area.getHeight();

	// Position each slider evenly within the available area
	for (int i = 0; i < count; ++i) {
		_sliders[i]->setBounds(area.getX() + (i * w), area.getY(), w, h);
	}
}