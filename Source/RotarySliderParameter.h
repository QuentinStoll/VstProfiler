/*
  ==============================================================================

    RotarySliderParameter.h
    Created: 6 Dec 2025 4:33:39pm
    Author:  Laurent ZHANG

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

struct RotarySliderParameter
{
	juce::String name;
	juce::String suffix;
	float minValue;
	float maxValue;
	float defaultValue;
};