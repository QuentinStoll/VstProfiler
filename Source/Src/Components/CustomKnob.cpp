#include "Components/CustomKnob.h"

#include "Stylesheet.h"

//==============================================================================
// CustomKnob Implementation
//==============================================================================

CustomKnob::CustomKnob(const juce::String& name, float min, float max, float defaultValue, const juce::String& suffix, float step) {
    _slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    _slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    _slider.setRange(min, max, step);
    _slider.setValue(defaultValue);
    _slider.setTextValueSuffix(" " + suffix);

    _label.setText(name, juce::dontSendNotification);
    _label.setJustificationType(juce::Justification::centred);
    _label.setColour(juce::Label::textColourId, ProfilerStyle::Colors::textMuted);
    _label.setMinimumHorizontalScale(0.7f);
    _label.setFont(ProfilerStyle::Fonts::regular(13.0f));

    addAndMakeVisible(_slider);
    addAndMakeVisible(_label);
}

void CustomKnob::paint(juce::Graphics& /*g*/) {
}

void CustomKnob::resized() {
    auto area = getLocalBounds();

    constexpr auto labelHeight = 16;
    constexpr auto labelToKnobGap = 2;

    const auto availableKnobHeight = juce::jmax(0, area.getHeight() - labelHeight - labelToKnobGap);
    const auto knobSide = juce::jmax(24, juce::jmin(68, juce::jmin(area.getWidth(), availableKnobHeight)));
    const auto contentHeight = juce::jmin(area.getHeight(), labelHeight + labelToKnobGap + knobSide);

    auto contentArea = area.withHeight(contentHeight).withCentre(area.getCentre());
    _slider.setBounds(contentArea.removeFromTop(knobSide).withSizeKeepingCentre(knobSide, knobSide));
    contentArea.removeFromTop(labelToKnobGap);
    _label.setBounds(contentArea.removeFromTop(labelHeight));
}
