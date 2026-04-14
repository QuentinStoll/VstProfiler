#include "RotarySlider.h"

RotarySlider::RotarySlider(const RotarySliderParameter& param) {
    addAndMakeVisible(_slider);
    _slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);

    _slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    _slider.setTextValueSuffix(param.suffix);
    _slider.setRange(param.minValue, param.maxValue, 0.01);
    _slider.setValue(param.defaultValue);

    addAndMakeVisible(_name);
    _name.setText(param.name, juce::dontSendNotification);
    _name.setJustificationType(juce::Justification::centred);
}

RotarySlider::~RotarySlider() {}

void RotarySlider::resized() {
    auto bounds = getLocalBounds();

    _name.setBounds(bounds.removeFromTop(20));

    auto sliderArea = bounds.reduced(5);
    _slider.setBounds(sliderArea);
}