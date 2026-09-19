#pragma once

#include <JuceHeader.h>

//==============================================================================
// CustomKnob Implementation
//==============================================================================
class CustomKnob : public juce::Component {
   public:
    CustomKnob(const juce::String& name, float min, float max, float defaultValue, const juce::String& suffix, float step = 0.1f);
    ~CustomKnob() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    juce::Slider& getSlider() { return _slider; };

   private:
    juce::Slider _slider;
    juce::Label _label;
};
