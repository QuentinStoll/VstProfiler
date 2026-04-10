#include "Components/CustomTickBox.h"

CustomTickBox::CustomTickBox(const juce::String name)
{
	setLookAndFeel(&_lookAndFeel);

	addAndMakeVisible(_toggleButton);
	_toggleButton.setButtonText(name);
}

CustomTickBox::~CustomTickBox()
{
	setLookAndFeel(nullptr);
}

void CustomTickBox::paint(juce::Graphics& g)
{
}

void CustomTickBox::resized()
{
	auto area = getLocalBounds();
	_toggleButton.setBounds(area);
}