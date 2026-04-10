#pragma once
#include <JuceHeader.h>

struct RotarySliderParameter {
    juce::String paramID;
    juce::String name;
    juce::String suffix;
    float minValue;
    float maxValue;
    float defaultValue;
};