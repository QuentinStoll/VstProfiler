/*
  ==============================================================================

    CloningScreen.h
    Created: 30 Nov 2025 1:39:39pm
    Author:  Laurent ZHANG

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class CloningScreen : public juce::Component
{
public:
	CloningScreen();
	~CloningScreen();

	void paint(juce::Graphics& g) override;
	void resized() override;

private:

	juce::TextButton _startCloneButton{ "Start Cloning" };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CloningScreen)
};