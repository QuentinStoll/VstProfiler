#include "Components/CustomToggleButton.h"

#include "Stylesheet.h"

void CustomToggleButtonLF::drawSwitch(juce::Graphics& g, juce::Rectangle<float> area,
                                      bool ticked, bool isEnabled, bool isMouseOver, bool isMouseDown) {
    area = area.reduced(1.0f);
    if (area.getWidth() <= 4.0f || area.getHeight() <= 4.0f) {
        return;
    }

    const auto enabledAlpha = isEnabled ? 1.0f : 0.35f;
    const auto trackH = juce::jmin(area.getHeight() * 0.7f, 24.0f);
    const auto trackW = juce::jmin(area.getWidth() * 0.86f, trackH * 2.65f);
    auto trackRect = area.withSizeKeepingCentre(trackW, trackH);
    const auto cornerSize = trackH * 0.5f;

    auto trackTop = ProfilerStyle::Colors::lightestGrey.darker(0.12f);
    auto trackBottom = ProfilerStyle::Colors::darkGrey.darker(0.24f);

    if (isMouseOver) {
        trackTop = trackTop.brighter(0.06f);
        trackBottom = trackBottom.brighter(0.04f);
    }

    if (isMouseDown) {
        trackTop = trackTop.darker(0.1f);
        trackBottom = trackBottom.darker(0.1f);
    }

    g.setGradientFill(ProfilerStyle::Gradients::vertical(trackRect, trackTop.withAlpha(enabledAlpha), trackBottom.withAlpha(enabledAlpha), 0.85f));
    g.fillRoundedRectangle(trackRect, cornerSize);

    g.setColour(ProfilerStyle::Colors::darkGrey.withAlpha(enabledAlpha * 0.7f));
    g.drawRoundedRectangle(trackRect, cornerSize, 1.0f);

    const auto thumbOverhang = juce::jmin(3.0f, area.getHeight() * 0.12f);
    const auto thumbW = trackH * 1.08f;
    const auto thumbH = juce::jmin(area.getHeight(), trackH + thumbOverhang * 2.0f);
    const auto thumbX = ticked ? trackRect.getRight() - thumbW + thumbOverhang : trackRect.getX() - thumbOverhang;
    auto thumbRect = juce::Rectangle<float>(thumbX, trackRect.getCentreY() - thumbH * 0.5f, thumbW, thumbH);
    const auto thumbCorner = juce::jmin(9.0f, thumbH * 0.3f);

    auto thumbColour = ticked ? ProfilerStyle::Colors::orange : ProfilerStyle::Colors::darkerGrey.brighter(0.08f);
    if (isMouseOver) {
        thumbColour = thumbColour.brighter(0.08f);
    }
    if (isMouseDown) {
        thumbColour = thumbColour.darker(0.12f);
    }

    g.setColour(ProfilerStyle::Colors::darkestGrey.withAlpha(enabledAlpha * 0.65f));
    g.fillRoundedRectangle(thumbRect.translated(0.0f, 1.5f), thumbCorner);

    g.setGradientFill(ProfilerStyle::Gradients::vertical(
        thumbRect,
        thumbColour.brighter(0.16f).withAlpha(enabledAlpha),
        thumbColour.darker(0.2f).withAlpha(enabledAlpha),
        0.8f));
    g.fillRoundedRectangle(thumbRect, thumbCorner);

    g.setColour(ProfilerStyle::Colors::darkerGrey.withAlpha(enabledAlpha * 0.75f));
    g.drawRoundedRectangle(thumbRect, thumbCorner, 1.0f);
}

//=============================================================================
// CustomToggleButtonLF Implementation
//=============================================================================

void CustomToggleButtonLF::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                            bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) {
    auto area = button.getLocalBounds().toFloat();
    const auto labelHeight = juce::jlimit(8.0f, 20.0f, area.getHeight() * 0.48f);
    auto labelArea = area.removeFromTop(labelHeight);
    auto switchArea = area;

    g.setColour(button.findColour(juce::ToggleButton::textColourId).withAlpha(button.isEnabled() ? 1.0f : 0.45f));
    g.setFont(juce::Font(juce::FontOptions().withHeight(juce::jlimit(8.0f, 15.0f, labelHeight * 0.8f))));
    g.drawFittedText(button.getButtonText(), labelArea.toNearestInt(), juce::Justification::centred, 1);

    drawSwitch(g, switchArea, button.getToggleState(), button.isEnabled(), shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
}

void CustomToggleButtonLF::drawTickBox(juce::Graphics& g, juce::Component& button,
                                       float x, float y, float w, float h,
                                       bool ticked, bool isEnabled, bool isMouseOver, bool isMouseDown) {
    juce::ignoreUnused(button);
    drawSwitch(g, juce::Rectangle<float>(x, y, w, h), ticked, isEnabled, isMouseOver, isMouseDown);
}

//=============================================================================
// CustomToggleButton Implementation
//=============================================================================

CustomToggleButton::CustomToggleButton(const juce::String& buttonText) : ToggleButton(buttonText) {
    setLookAndFeel(&_customLF);
}

CustomToggleButton::~CustomToggleButton() {
    setLookAndFeel(nullptr);
}

void CustomToggleButton::paint(juce::Graphics& g) {
    ToggleButton::paint(g);
}

void CustomToggleButton::resized() {
    ToggleButton::resized();
}
