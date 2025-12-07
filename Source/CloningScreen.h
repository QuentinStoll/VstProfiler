/*
  ==============================================================================

    CloningScreen.h
    Created: 30 Nov 2025 1:39:39pm
    Author:  Laurent ZHANG

  ==============================================================================
*/

#pragma once
#include "PluginProcessor.h"
#include <JuceHeader.h>

class CloningScreen : public juce::Component
{
public:
	CloningScreen(ProfilerAudioProcessor& p);
	~CloningScreen();

	void paint(juce::Graphics& g) override;
	void resized() override;

private:
	ProfilerAudioProcessor& _audioProcessor;

	juce::TextButton _sweepButton{ "Start sweep" };
	juce::TextButton _irButton{ "load ir" };
	juce::TextButton _ampButton{ "load amp" };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CloningScreen)
};