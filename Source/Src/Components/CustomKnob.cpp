#include "Components/CustomKnob.h"

#include "Styles/Stylesheet.h"

//==============================================================================
// CustomKnobLF Implementation
//==============================================================================

void CustomKnobLF::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                    float sliderPos, const float rotaryStartAngle,
                                    const float rotaryEndAngle, juce::Slider& slider) {
    // Customize the rotary slider's appearance
    auto fill = ProfilerStyle::Colors::orange;
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(10);
    auto radius = std::min(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    auto lineW = 3.0f;
    auto centreX = x + width * 0.5f;
    auto centreY = y + height * 0.5f;
    auto arcRadius = radius - lineW * 2.0f;

    // Draw the background arc (the full range of the slider)
    juce::Path backgroundArc;
    backgroundArc.addCentredArc(centreX, centreY, arcRadius, arcRadius, 0.0f,
                                rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(ProfilerStyle::Colors::darkGrey);
    g.strokePath(backgroundArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Only draw the value arc if the slider is enabled
    if (slider.isEnabled()) {
        juce::Path valueArc;
        valueArc.addCentredArc(centreX, centreY, arcRadius, arcRadius, 0.0f,
                               rotaryStartAngle, toAngle, true);
        g.setColour(fill);
        g.strokePath(valueArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Style the knob (a circle in the center of the arc)
    auto knobRadius = arcRadius - 6.0f;
    juce::Rectangle<float> knobBounds(centreX - knobRadius, centreY - knobRadius,
                                      knobRadius * 2, knobRadius * 2);

    // Create a vertical gradient for the knob
    juce::ColourGradient knobGrad(ProfilerStyle::Colors::lighterGrey, centreX, centreY - knobRadius,
                                  ProfilerStyle::Colors::darkestGrey, centreX, centreY + knobRadius, false);
    g.setGradientFill(knobGrad);
    g.fillEllipse(knobBounds);

    // Draw the outline of the knob
    g.setColour(ProfilerStyle::Colors::darkerGrey);
    g.drawEllipse(knobBounds, 1.0f);

    // Draw the indicator dot on the knob
    const float dotRadius = knobRadius * 0.08f;
    const float dotDist = knobRadius * 0.72f;
    const float dotX = centreX + dotDist * std::sin(toAngle);
    const float dotY = centreY - dotDist * std::cos(toAngle);

    g.setColour(ProfilerStyle::Colors::orange);
    g.fillEllipse(dotX - dotRadius, dotY - dotRadius,
                  dotRadius * 2.f, dotRadius * 2.f);
}

void CustomKnobLF::drawLabel(juce::Graphics& g, juce::Label& label) {
    // Custom label drawing (e.g., for the slider's text box)
    auto area = label.getLocalBounds().toFloat();
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(juce::FontOptions().withHeight(14.0f)));
    g.drawFittedText(label.getText(), area.toNearestInt(), juce::Justification::centred, 1);
}

//==============================================================================
// CustomKnob Implementation
//==============================================================================

CustomKnob::CustomKnob(const juce::String& name, float min, float max, float defaultValue, const juce::String& suffix, float step) {
    //
    setLookAndFeel(&_customLF);

    //
    _slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    _slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    _slider.setRange(min, max, step);
    _slider.setValue(defaultValue);
    _slider.setTextValueSuffix(" " + suffix);

    //
    _label.setText(name, juce::dontSendNotification);
    _label.setJustificationType(juce::Justification::centred);
    _label.setColour(juce::Label::textColourId, juce::Colours::white);

    //
    addAndMakeVisible(_slider);
    addAndMakeVisible(_label);
}

CustomKnob::~CustomKnob() {
    setLookAndFeel(nullptr);
}

void CustomKnob::paint(juce::Graphics& g) {
}

void CustomKnob::resized() {
    auto area = getLocalBounds();
    _label.setBounds(area.removeFromTop(20));
    _slider.setBounds(area);
}