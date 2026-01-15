#pragma once
#include <JuceHeader.h>
#include "RotarySliderParameter.h"

class RotarySlider : public juce::Component
{
public:
	RotarySlider(const RotarySliderParameter& param);
	~RotarySlider();
	
	void resized() override;

	juce::Slider& getSlider() { return _slider; };

private:
    juce::Slider _slider;
	juce::Label _name;
};

