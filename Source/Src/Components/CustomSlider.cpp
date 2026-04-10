#include "Components/CustomSlider.h"

CustomSlider::CustomSlider(const juce::String& name, float min, float max, float defaultValue, const juce::String& suffix, float step)
{
	//
	setLookAndFeel(&_lookAndFeel);

	//
	_slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
	_slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
	_slider.setRange(min, max, step);
	_slider.setValue(defaultValue);
	_slider.setTextValueSuffix(" " + suffix);

	//
	_label.setText(name, juce::dontSendNotification);
	_label.setJustificationType(juce::Justification::centred);
	_label.setColour(juce::Label::textColourId, juce::Colours::white);

	//
	addAndMakeVisible(_slider);
	addAndMakeVisible(_label);
}

CustomSlider::~CustomSlider()
{
	setLookAndFeel(nullptr);
}

void CustomSlider::paint(juce::Graphics& g)
{
}

void CustomSlider::resized()
{
	auto area = getLocalBounds();
	_label.setBounds(area.removeFromTop(20));
	_slider.setBounds(area);
}