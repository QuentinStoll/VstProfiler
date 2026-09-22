#pragma once

#include <JuceHeader.h>

#include "Stylesheet.h"

//=============================================================================
// CustomTextButton Implementation
//=============================================================================

class CustomTextButton : public juce::TextButton {
   public:
    CustomTextButton(const juce::String& buttonName,
                     ProfilerStyle::Theme theme = ProfilerStyle::Theme::Dark);
    ~CustomTextButton() override = default;

    void setTheme(ProfilerStyle::Theme theme);
    void setOutlineVisible(bool shouldShowOutline);
    void setOutlineColour(juce::Colour outlineColour);
};
