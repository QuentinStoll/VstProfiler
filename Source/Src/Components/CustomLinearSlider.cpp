#include "Components/CustomLinearSlider.h"

#include "Stylesheet.h"

//==============================================================================
// CustomLinearSliderLF Implementation
//==============================================================================

void CustomLinearSliderLF::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                            float sliderPos, float minSliderPos, float maxSliderPos,
                                            const juce::Slider::SliderStyle style, juce::Slider& slider) {
    (void)minSliderPos;
    (void)maxSliderPos;

    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(8.0f);
    if (bounds.getWidth() <= 0.0f || bounds.getHeight() <= 0.0f) {
        return;
    }

    const auto isVertical = style == juce::Slider::LinearVertical || style == juce::Slider::LinearBarVertical;
    const auto enabledAlpha = slider.isEnabled() ? 1.0f : 0.35f;
    const auto trackThickness = 8.0f;

    juce::Rectangle<float> trackBounds;
    if (isVertical) {
        trackBounds = bounds.withSizeKeepingCentre(trackThickness, bounds.getHeight());
    } else {
        trackBounds = bounds.withSizeKeepingCentre(bounds.getWidth(), trackThickness);
    }

    g.setColour(ProfilerStyle::Colors::darkGrey.withAlpha(enabledAlpha));
    g.fillRoundedRectangle(trackBounds, trackThickness * 0.5f);

    g.setColour(ProfilerStyle::Colors::darkerGrey.withAlpha(enabledAlpha));
    g.drawRoundedRectangle(trackBounds, trackThickness * 0.5f, 1.0f);

    const auto clampedSliderPos = isVertical
                                      ? juce::jlimit(trackBounds.getY(), trackBounds.getBottom(), sliderPos)
                                      : juce::jlimit(trackBounds.getX(), trackBounds.getRight(), sliderPos);

    if (slider.isEnabled()) {
        juce::Rectangle<float> valueBounds;
        if (isVertical) {
            valueBounds = juce::Rectangle<float>(trackBounds.getX(), clampedSliderPos,
                                                 trackBounds.getWidth(), trackBounds.getBottom() - clampedSliderPos);
        } else {
            valueBounds = juce::Rectangle<float>(trackBounds.getX(), trackBounds.getY(),
                                                 clampedSliderPos - trackBounds.getX(), trackBounds.getHeight());
        }

        if (!valueBounds.isEmpty()) {
            const juce::ColourGradient valueGradient(
                ProfilerStyle::Colors::orange.brighter(0.25f), valueBounds.getX(), valueBounds.getY(),
                ProfilerStyle::Colors::orange.darker(0.35f), valueBounds.getRight(), valueBounds.getBottom(), false);

            g.setGradientFill(valueGradient);
            g.fillRoundedRectangle(valueBounds, trackThickness * 0.5f);
        }
    }

    const auto thumbWidth = isVertical ? 22.0f : 12.0f;
    const auto thumbHeight = isVertical ? 12.0f : 22.0f;
    const auto thumbCorner = 5.0f;
    juce::Rectangle<float> thumbBounds(
        isVertical ? bounds.getCentreX() - thumbWidth * 0.5f : clampedSliderPos - thumbWidth * 0.5f,
        isVertical ? clampedSliderPos - thumbHeight * 0.5f : bounds.getCentreY() - thumbHeight * 0.5f,
        thumbWidth,
        thumbHeight);

    juce::ColourGradient thumbGradient(
        ProfilerStyle::Colors::lighterGrey, thumbBounds.getCentreX(), thumbBounds.getY(),
        ProfilerStyle::Colors::darkestGrey, thumbBounds.getCentreX(), thumbBounds.getBottom(), false);

    g.setGradientFill(thumbGradient);
    g.fillRoundedRectangle(thumbBounds, thumbCorner);

    g.setColour(ProfilerStyle::Colors::darkerGrey.withAlpha(enabledAlpha));
    g.drawRoundedRectangle(thumbBounds, thumbCorner, 1.0f);

    g.setColour(ProfilerStyle::Colors::orange.withAlpha(enabledAlpha));
    if (isVertical) {
        g.drawLine(thumbBounds.getX() + 5.0f, thumbBounds.getCentreY(),
                   thumbBounds.getRight() - 5.0f, thumbBounds.getCentreY(), 2.0f);
    } else {
        g.drawLine(thumbBounds.getCentreX(), thumbBounds.getY() + 5.0f,
                   thumbBounds.getCentreX(), thumbBounds.getBottom() - 5.0f, 2.0f);
    }
}

void CustomLinearSliderLF::drawLabel(juce::Graphics& g, juce::Label& label) {
    auto area = label.getLocalBounds().toFloat();
    g.setColour(label.findColour(juce::Label::textColourId));
    g.setFont(juce::Font(juce::FontOptions().withHeight(14.0f)));
    g.drawFittedText(label.getText(), area.toNearestInt(), label.getJustificationType(), 1);
}

//==============================================================================
// CustomLinearSlider Implementation
//==============================================================================

CustomLinearSlider::CustomLinearSlider(const juce::String& name, float min, float max, float defaultValue, const juce::String& suffix,
                                       float step, juce::Slider::SliderStyle sliderStyle) {
    setLookAndFeel(&_customLF);

    _slider.setSliderStyle(sliderStyle);
    _slider.setTextBoxStyle(isHorizontal() ? juce::Slider::TextBoxRight : juce::Slider::TextBoxBelow, false, 58, 20);
    _slider.setRange(min, max, step);
    _slider.setValue(defaultValue);
    _slider.setTextValueSuffix(" " + suffix);

    _label.setText(name, juce::dontSendNotification);
    _label.setJustificationType(isHorizontal() ? juce::Justification::centredLeft : juce::Justification::centred);
    _label.setColour(juce::Label::textColourId, juce::Colours::white);

    addAndMakeVisible(_slider);
    addAndMakeVisible(_label);
}

CustomLinearSlider::~CustomLinearSlider() {
    setLookAndFeel(nullptr);
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
