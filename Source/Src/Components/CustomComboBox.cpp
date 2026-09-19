#include "Components/CustomComboBox.h"

#include "Stylesheet.h"

CustomComboBox::CustomComboBox() {
    setColour(juce::ComboBox::textColourId, ProfilerStyle::Colors::text);
    setColour(juce::ComboBox::backgroundColourId, ProfilerStyle::Colors::elevated);
}
