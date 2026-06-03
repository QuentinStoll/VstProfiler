#pragma once

#include <JuceHeader.h>

#include "Stylesheet.h"

//=============================================================================
// CustomImageButton Implementation
//=============================================================================

class CustomImageButton : public juce::Button {
   public:
    CustomImageButton(const juce::String& buttonName,
                      juce::Image image = {},
                      ProfilerStyle::Theme theme = ProfilerStyle::Theme::Dark);

    CustomImageButton(const juce::String& buttonName,
                      std::unique_ptr<juce::Drawable> drawable = nullptr,
                      ProfilerStyle::Theme theme = ProfilerStyle::Theme::Dark);

    CustomImageButton(const juce::String& buttonName,
                      const juce::File& svgFile,
                      ProfilerStyle::Theme theme = ProfilerStyle::Theme::Dark);

    ~CustomImageButton() override = default;

    void setTheme(ProfilerStyle::Theme theme);

    void setOutlineVisible(bool shouldShowOutline) { _outlineVisible = shouldShowOutline; }
    void setOutlineColour(juce::Colour outlineColour) { _outlineColour = outlineColour; }

    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override;

   private:
    juce::Image _image;
    std::unique_ptr<juce::Drawable> _drawable;

    // Colors
    juce::Colour _backgroundColour = ProfilerStyle::Colors::darkerGrey;
    juce::Colour _outlineColour = ProfilerStyle::Colors::lightestGrey;
    juce::Colour _textColour = ProfilerStyle::Colors::white;

    // States
    bool _outlineVisible = false;

    // Methods
    bool hasImageContent() const { return _image.isValid() || _drawable != nullptr; };
    void setDrawableColourToWhite();
    void drawImageContent(juce::Graphics& g, juce::Rectangle<float> imageArea, float alpha);
};
