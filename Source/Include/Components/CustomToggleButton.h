#pragma once

#include <JuceHeader.h>

//=============================================================================
// CustomToggleButtonLF Implementation
//=============================================================================

class CustomToggleButtonLF : public juce::LookAndFeel_V4 {
   public:
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawTickBox(juce::Graphics& g, juce::Component& button,
                     float x, float y, float w, float h,
                     bool ticked, bool isEnabled, bool isMouseOver, bool isMouseDown) override;

   private:
    static void drawSwitch(juce::Graphics& g, juce::Rectangle<float> area,
                           bool ticked, bool isEnabled, bool isMouseOver, bool isMouseDown);
};

//=============================================================================
// CustomToggleButton Implementation
//=============================================================================

class CustomToggleButton : public juce::ToggleButton {
   public:
    CustomToggleButton(const juce::String& buttonText);
    ~CustomToggleButton();

    void paint(juce::Graphics& g) override;
    void resized() override;

   private:
    CustomToggleButtonLF _customLF;
};
