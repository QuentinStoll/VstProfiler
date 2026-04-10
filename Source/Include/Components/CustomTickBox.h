#pragma once

#include <JuceHeader.h>
#include "Styles/CustomTickBoxLF.h"

class CustomTickBox : public juce::Component
{
public:
	CustomTickBox(const juce::String name);
	~CustomTickBox() override;

	void paint(juce::Graphics& g) override;
	void resized() override;

	void setToggleState(bool shouldBeOn) { _toggleButton.setToggleState(shouldBeOn, juce::dontSendNotification); }

private:
	CustomTickBoxLF _lookAndFeel;
	juce::ToggleButton _toggleButton;
};