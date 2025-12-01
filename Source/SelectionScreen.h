/*
  ==============================================================================

    SelectionScreen.h
    Created: 30 Nov 2025 2:11:10pm
    Author:  Laurent ZHANG

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <functional>
#include "ScreenID.h"

class SelectionScreen : public juce::Component
{
public:
	using ScreenSelectedCallback = std::function<void(ScreenID)>;

	SelectionScreen(ScreenSelectedCallback callback);
	~SelectionScreen();

	void paint(juce::Graphics& g) override;
	void resized() override;

private:
	juce::TextButton cloneButton{ "Clone" };
	juce::TextButton useButton{ "Use" };

	ScreenSelectedCallback screenSelectedCallback;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SelectionScreen)
};