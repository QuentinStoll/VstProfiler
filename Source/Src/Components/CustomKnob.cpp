#include "Components/CustomKnob.h"

#include "Stylesheet.h"

//==============================================================================
// CustomKnobLF Implementation
//==============================================================================

void CustomKnobLF::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                    float sliderPos, const float rotaryStartAngle,
                                    const float rotaryEndAngle, juce::Slider& slider) {
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(6.0f);
    const auto side = std::min(bounds.getWidth(), bounds.getHeight());
    auto knobArea = bounds.withSizeKeepingCentre(side, side);
    const auto centre = knobArea.getCentre();
    const auto radius = side * 0.5f;
    const auto enabledAlpha = slider.isEnabled() ? 1.0f : 0.35f;
    const auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const auto lineW = juce::jlimit(2.0f, 4.0f, radius * 0.08f);
    const auto arcRadius = radius - lineW * 1.5f;
    const auto knobRadius = arcRadius - lineW * 2.4f;

    juce::Path backgroundArc;
    backgroundArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                                rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(ProfilerStyle::Colors::darkGrey.withAlpha(enabledAlpha));
    g.strokePath(backgroundArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    if (slider.isEnabled()) {
        juce::Path valueArc;
        valueArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                               rotaryStartAngle, toAngle, true);
        juce::ColourGradient valueGradient(
            ProfilerStyle::Colors::orange.brighter(0.25f), centre.x - arcRadius, centre.y,
            ProfilerStyle::Colors::orange.darker(0.35f), centre.x + arcRadius, centre.y, false);
        g.setGradientFill(valueGradient);
        g.strokePath(valueArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    juce::Rectangle<float> knobBounds(centre.x - knobRadius, centre.y - knobRadius,
                                      knobRadius * 2.0f, knobRadius * 2.0f);

    g.setColour(ProfilerStyle::Colors::darkestGrey.withAlpha(enabledAlpha * 0.7f));
    g.fillEllipse(knobBounds.translated(0.0f, 2.0f).expanded(1.0f));

    juce::ColourGradient knobGrad(
        ProfilerStyle::Colors::lighterGrey.withAlpha(enabledAlpha), centre.x, centre.y - knobRadius,
        ProfilerStyle::Colors::darkestGrey.withAlpha(enabledAlpha), centre.x, centre.y + knobRadius, false);
    g.setGradientFill(knobGrad);
    g.fillEllipse(knobBounds);

    g.setColour(ProfilerStyle::Colors::darkerGrey.withAlpha(enabledAlpha));
    g.drawEllipse(knobBounds, 1.0f);

    auto capBounds = knobBounds.reduced(knobRadius * 0.28f);
    g.setColour(ProfilerStyle::Colors::darkGrey.withAlpha(enabledAlpha * 0.55f));
    g.fillEllipse(capBounds);

    const auto indicatorStart = knobRadius * 0.18f;
    const auto indicatorEnd = knobRadius * 0.76f;
    const auto startX = centre.x + indicatorStart * std::sin(toAngle);
    const auto startY = centre.y - indicatorStart * std::cos(toAngle);
    const auto endX = centre.x + indicatorEnd * std::sin(toAngle);
    const auto endY = centre.y - indicatorEnd * std::cos(toAngle);

    g.setColour(ProfilerStyle::Colors::orange.withAlpha(enabledAlpha));
    g.drawLine(startX, startY, endX, endY, juce::jlimit(2.0f, 3.0f, knobRadius * 0.12f));
    g.fillEllipse(endX - 2.5f, endY - 2.5f, 5.0f, 5.0f);
}

void CustomKnobLF::drawLabel(juce::Graphics& g, juce::Label& label) {
    auto area = label.getLocalBounds().toFloat();
    g.setColour(label.findColour(juce::Label::textColourId));
    g.setFont(juce::Font(juce::FontOptions().withHeight(14.0f)));
    g.drawFittedText(label.getText(), area.toNearestInt(), label.getJustificationType(), 1);
}

//==============================================================================
// CustomKnob Implementation
//==============================================================================

CustomKnob::CustomKnob(const juce::String& name, float min, float max, float defaultValue, const juce::String& suffix, float step) {
    setLookAndFeel(&_customLF);

    _slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    _slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 16);
    _slider.setRange(min, max, step);
    _slider.setValue(defaultValue);
    _slider.setTextValueSuffix(" " + suffix);

    _label.setText(name, juce::dontSendNotification);
    _label.setJustificationType(juce::Justification::centred);
    _label.setColour(juce::Label::textColourId, juce::Colours::white);

    addAndMakeVisible(_slider);
    addAndMakeVisible(_label);
}

CustomKnob::~CustomKnob() {
    setLookAndFeel(nullptr);
}

void CustomKnob::paint(juce::Graphics& /*g*/) {
}

void CustomKnob::resized() {
    auto area = getLocalBounds();

    constexpr auto labelHeight = 18;
    constexpr auto labelToKnobGap = 2;
    constexpr auto valueHeight = 16;

    const auto availableKnobHeight = juce::jmax(0, area.getHeight() - labelHeight - labelToKnobGap - valueHeight);
    const auto knobSide = juce::jmax(24, juce::jmin(area.getWidth(), availableKnobHeight));
    const auto sliderHeight = knobSide + valueHeight;
    const auto contentHeight = juce::jmin(area.getHeight(), labelHeight + labelToKnobGap + sliderHeight);

    auto contentArea = area.withHeight(contentHeight).withCentre(area.getCentre());
    _label.setBounds(contentArea.removeFromTop(labelHeight));
    contentArea.removeFromTop(labelToKnobGap);
    _slider.setBounds(contentArea.removeFromTop(sliderHeight).withSizeKeepingCentre(knobSide, sliderHeight));
}
