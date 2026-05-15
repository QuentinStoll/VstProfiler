#pragma once

#include <JuceHeader.h>

#include "Styles/CustomButtonLF.h"

class CustomSlider : public juce::Component {
   public:
    CustomSlider(const juce::String& name, float min, float max, float defaultValue, const juce::String& suffix, float step = 0.1f);
    ~CustomSlider();

    void paint(juce::Graphics& g);
    void resized();

    juce::Slider& getSlider() { return _slider; };

   private:
    CustomButtonLF _lookAndFeel;

    juce::Slider _slider;
    juce::Label _label;
};