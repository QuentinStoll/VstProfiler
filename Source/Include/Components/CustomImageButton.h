#pragma once

#include <JuceHeader.h>

#include "Stylesheet.h"

//=============================================================================
// CustomImageButton Implementation
//=============================================================================

class CustomImageButton : public juce::Button {
   public:
    CustomImageButton(const juce::String& buttonName,
                      const void* binaryData = nullptr,
                      size_t dataSize = 0,
                      ProfilerStyle::Theme theme = ProfilerStyle::Theme::Dark);

    ~CustomImageButton() override = default;

    void setTheme(ProfilerStyle::Theme theme);

    void setOutlineVisible(bool shouldShowOutline) { _outlineVisible = shouldShowOutline; }
    void setOutlineColour(juce::Colour outlineColour) { _outlineColour = outlineColour; }

    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override;

   private:
    std::unique_ptr<juce::Drawable> _drawable;

    // Colors
    juce::Colour _backgroundColour = ProfilerStyle::Colors::darkerGrey;
    juce::Colour _outlineColour = ProfilerStyle::Colors::lightestGrey;
    juce::Colour _textColour = ProfilerStyle::Colors::white;

    // States
    bool _outlineVisible = false;

    // Methods
    void drawImageContent(juce::Graphics& g, juce::Rectangle<float> imageArea, float alpha);
};
