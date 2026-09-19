#pragma once

#include <JuceHeader.h>

//==============================================================================
// CustomLinearSlider Implementation
//==============================================================================
class CustomLinearSlider : public juce::Component {
   public:
    CustomLinearSlider(const juce::String& name, float min, float max, float defaultValue, const juce::String& suffix,
                       float step = 0.1f, juce::Slider::SliderStyle sliderStyle = juce::Slider::LinearVertical);
    ~CustomLinearSlider() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    juce::Slider& getSlider() { return _slider; };

   private:
    bool isHorizontal() const;

    juce::Slider _slider;
    juce::Label _label;
};
