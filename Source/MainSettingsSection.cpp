/*
  ==============================================================================

    MainSettingsSection.cpp
    Created: 1 Dec 2025 11:39:55pm
    Author:  Laurent ZHANG

  ==============================================================================
*/

#include "MainSettingsSection.h"

MainSettingsSection::MainSettingsSection(juce::AudioProcessorValueTreeState& apvts)
{
	// Define the parameters for each rotary slider
	std::vector<RotarySliderParameter> sliderParams = {
		{"gain", "Gain", " dB", -24.0f, 24.0f, 0.0f },
		{"gate", "Gate" , "", 0.0f, 10.0f, 0.0f },
		{"bass", "Bass", " dB", -24.0f, 24.0f, 0.0f },
		{"mid", "Mid", " dB", -12.0f, 12.0f, 0.0f },
		{"treble", "Treble", " dB", -24.0f, 24.0f, 0.0f },
		{"volume", "Volume", " %", 0.0f, 1.0f, 1.0f }
	};

	// Create and add the rotary sliders based on the defined parameters
	for (const auto& param : sliderParams)
	{
		auto slider = std::make_unique<RotarySlider>(param);
		_sliders.push_back(std::move(slider));
		addAndMakeVisible(_sliders.back().get());

		auto attachment = std::make_unique<SliderAttachment>(
			apvts,
			param.paramID,
			_sliders.back()->getSlider()
		);
		_attachments.push_back(std::move(attachment));
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