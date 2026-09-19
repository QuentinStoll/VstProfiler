#pragma once

#include <JuceHeader.h>

//=============================================================================
// CustomToggleButton Implementation
//=============================================================================

class CustomToggleButton : public juce::ToggleButton {
   public:
    CustomToggleButton(const juce::String& buttonText);
    ~CustomToggleButton() override = default;

    void setLabelVisible(bool shouldShowLabel);
};
