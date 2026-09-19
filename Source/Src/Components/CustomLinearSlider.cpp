#include "Components/CustomLinearSlider.h"

#include "Stylesheet.h"

//==============================================================================
// CustomLinearSlider Implementation
//==============================================================================

CustomLinearSlider::CustomLinearSlider(const juce::String& name, float min, float max, float defaultValue, const juce::String& suffix,
                                       float step, juce::Slider::SliderStyle sliderStyle) {
    _slider.setSliderStyle(sliderStyle);
    _slider.setTextBoxStyle(isHorizontal() ? juce::Slider::TextBoxRight : juce::Slider::TextBoxBelow, false, 58, 20);
    _slider.setRange(min, max, step);
    _slider.setValue(defaultValue);
    _slider.setTextValueSuffix(" " + suffix);

    _label.setText(name, juce::dontSendNotification);
    _label.setJustificationType(isHorizontal() ? juce::Justification::centredLeft : juce::Justification::centred);
    _label.setColour(juce::Label::textColourId, ProfilerStyle::Colors::text);
    _label.setFont(ProfilerStyle::Fonts::regular(14.0f));

    addAndMakeVisible(_slider);
    addAndMakeVisible(_label);
}

void CustomLinearSlider::paint(juce::Graphics& /*g*/) {
}

void CustomLinearSlider::resized() {
    auto area = getLocalBounds();

    if (isHorizontal()) {
        const auto labelWidth = juce::jlimit(70, 120, _label.getText().length() * 7 + 8);
        _label.setBounds(area.removeFromLeft(labelWidth));
        area.removeFromLeft(4);
        _slider.setBounds(area);
        return;
    }

    _label.setBounds(area.removeFromTop(20));
    _slider.setBounds(area);
}

bool CustomLinearSlider::isHorizontal() const {
    const auto style = _slider.getSliderStyle();
    return style == juce::Slider::LinearHorizontal ||
           style == juce::Slider::LinearBar ||
           style == juce::Slider::TwoValueHorizontal ||
           style == juce::Slider::ThreeValueHorizontal;
}
