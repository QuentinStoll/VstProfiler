#pragma once

#include <JuceHeader.h>

//==============================================================================
// CustomLinearSliderLF Implementation
//==============================================================================

class CustomLinearSliderLF : public juce::LookAndFeel_V4 {
   public:
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          const juce::Slider::SliderStyle style, juce::Slider& slider) override;

    void drawLabel(juce::Graphics& g, juce::Label& label) override;
};

//==============================================================================
// CustomLinearSlider Implementation
//==============================================================================
class CustomLinearSlider : public juce::Component {
   public:
    CustomLinearSlider(const juce::String& name, float min, float max, float defaultValue, const juce::String& suffix,
                       float step = 0.1f, juce::Slider::SliderStyle sliderStyle = juce::Slider::LinearVertical);
    ~CustomLinearSlider();

    void paint(juce::Graphics& g) override;
    void resized() override;

    juce::Slider& getSlider() { return _slider; };

   private:
    bool isHorizontal() const;

    CustomLinearSliderLF _customLF;

    juce::Slider _slider;
    juce::Label _label;
};
