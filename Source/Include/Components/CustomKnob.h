#pragma once

#include <JuceHeader.h>

//==============================================================================
// CustomKnobLF Implementation
//==============================================================================

class CustomKnobLF : public juce::LookAndFeel_V4 {
   public:
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, const float rotaryStartAngle,
                          const float rotaryEndAngle, juce::Slider& slider) override;

    void drawLabel(juce::Graphics& g, juce::Label& label) override;
};

//==============================================================================
// CustomKnob Implementation
//==============================================================================
class CustomKnob : public juce::Component {
   public:
    CustomKnob(const juce::String& name, float min, float max, float defaultValue, const juce::String& suffix, float step = 0.1f);
    ~CustomKnob();

    void paint(juce::Graphics& g);
    void resized();

    juce::Slider& getSlider() { return _slider; };

   private:
    CustomKnobLF _customLF;

    juce::Slider _slider;
    juce::Label _label;
};