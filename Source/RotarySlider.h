/*
  ==============================================================================

    RotarySlider.h
    Created: 2 Dec 2025 2:28:12pm
    Author:  Laurent ZHANG

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "RotarySliderParameter.h"

class RotarySlider : public juce::Component
{
public:
	RotarySlider(const RotarySliderParameter& param);
	~RotarySlider();
	
	void resized() override;

private:
    juce::Slider _slider;
	juce::Label _name;
};

