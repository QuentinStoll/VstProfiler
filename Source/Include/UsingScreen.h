/*
  ==============================================================================

    UsingScreen.h
    Created: 30 Nov 2025 2:06:33pm
    Author:  Laurent ZHANG

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "MainSettingsSection.h"
#include "EqNormalizeSection.h"
#include "AmpLoaderSection.h"

class UsingScreen : public juce::Component
{
public:
	UsingScreen(juce::AudioProcessorValueTreeState& apvts);
	~UsingScreen();

	void paint(juce::Graphics& g) override;
	void resized() override;

private:
	// Main Settings section
	std::unique_ptr<MainSettingsSection> _mainSettingsSection;
	
	// EQ and Normalize section
	std::unique_ptr<EqNormalizeSection> _eqNormalizeSection;
	
	// Amp Loader section
	std::unique_ptr<AmpLoaderSection> _ampLoaderSection;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UsingScreen)
};

