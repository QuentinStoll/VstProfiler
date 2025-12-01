/*
  ==============================================================================

    UsingScreen.h
    Created: 30 Nov 2025 2:06:33pm
    Author:  Laurent ZHANG

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class UsingScreen : public juce::Component
{
public:
	UsingScreen();
	~UsingScreen();

	void paint(juce::Graphics& g) override;
	void resized() override;

private:

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UsingScreen)
};

