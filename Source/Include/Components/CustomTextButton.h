#pragma once

#include <JuceHeader.h>

#include "Stylesheet.h"

//=============================================================================
// CustomTextButtonLF Implementation
//=============================================================================

class CustomTextButtonLF : public juce::LookAndFeel_V4 {
   public:
    void setOutlineVisible(bool shouldShowOutline);
    void setOutlineColour(juce::Colour outlineColour);

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool isMouseOverButton, bool isButtonDown) override;

    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
                        bool isMouseOverButton, bool isButtonDown) override;

   private:
    bool _outlineVisible = false;
    juce::Colour _outlineColour = ProfilerStyle::Colors::lightestGrey;
};

//=============================================================================
// CustomTextButton Implementation
//=============================================================================

class CustomTextButton : public juce::TextButton {
   public:
    CustomTextButton(const juce::String& buttonName,
                     ProfilerStyle::Theme theme = ProfilerStyle::Theme::Dark);
    ~CustomTextButton() override;

    void setTheme(ProfilerStyle::Theme theme);
    void setOutlineVisible(bool shouldShowOutline);
    void setOutlineColour(juce::Colour outlineColour);

    void paint(juce::Graphics& g) override;
    void resized() override;

   private:
    CustomTextButtonLF _customLF;
};
