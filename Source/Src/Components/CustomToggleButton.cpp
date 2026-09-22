#include "Components/CustomToggleButton.h"

#include "Stylesheet.h"

CustomToggleButton::CustomToggleButton(const juce::String& buttonText) : ToggleButton(buttonText) {
    setColour(juce::ToggleButton::textColourId, ProfilerStyle::Colors::text);
}

void CustomToggleButton::setLabelVisible(bool shouldShowLabel) {
    getProperties().set(ProfilerStyle::Properties::toggleLabelVisible, shouldShowLabel);
    repaint();
}
