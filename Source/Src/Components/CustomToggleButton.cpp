#include "Components/CustomToggleButton.h"

#include "Stylesheet.h"

//=============================================================================
// CustomToggleButtonLF Implementation
//=============================================================================

void CustomToggleButtonLF::drawTickBox(juce::Graphics& g, juce::Component& button,
                                       float /*x*/, float /*y*/, float /*w*/, float /*h*/,
                                       bool ticked, bool /*isEnabled*/, bool /*isMouseOver*/, bool /*isMouseDown*/) {
    auto area = button.getLocalBounds().toFloat();

    auto textArea = area.removeFromTop(area.getHeight() * 0.5f);
    auto switchArea = area;

    // Switch part
    float trackW = switchArea.getWidth() * 0.6f;
    float trackH = switchArea.getHeight() * 0.8f;
    auto trackRect = switchArea.withSizeKeepingCentre(trackW, trackH);
    auto cornerSize = trackH * 0.5f;

    g.setColour(ProfilerStyle::Colors::darkGrey);
    g.fillRoundedRectangle(trackRect, cornerSize);

    // Cursor part
    float thumbSize = switchArea.getHeight();
    float thumbX = ticked ? (trackRect.getRight() - thumbSize) : trackRect.getX();

    juce::Rectangle<float> thumbRect(thumbX,
                                     trackRect.getCentreY() - (thumbSize * 0.5f),
                                     thumbSize, thumbSize);

    auto cursorColor = ticked ? ProfilerStyle::Colors::orange : ProfilerStyle::Colors::darkerGrey;

    g.setGradientFill(ProfilerStyle::Gradients::vertical(
        thumbRect,
        cursorColor.darker(0.2f),
        cursorColor,
        0.9f));
    g.fillRoundedRectangle(thumbRect, thumbSize * 0.4f);
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