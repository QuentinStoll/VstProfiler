#include "Components/CustomTextButton.h"

//=============================================================================
// CustomTextButtonLF Implementation
//=============================================================================

void CustomTextButtonLF::setOutlineVisible(bool shouldShowOutline) {
    _outlineVisible = shouldShowOutline;
}

void CustomTextButtonLF::setOutlineColour(juce::Colour outlineColour) {
    _outlineColour = outlineColour;
}

void CustomTextButtonLF::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                              const juce::Colour& backgroundColour,
                                              bool isMouseOverButton, bool isButtonDown) {
    auto area = button.getLocalBounds().toFloat();
    auto cornerSize = 4.0f;
    auto fillArea = _outlineVisible ? area.reduced(2.0f) : area;

    auto baseColour = backgroundColour;
    if (isButtonDown)
        baseColour = baseColour.darker(0.2f);
    else if (isMouseOverButton)
        baseColour = baseColour.brighter(0.1f);

    g.setColour(baseColour);
    g.fillRoundedRectangle(fillArea, cornerSize);

    if (_outlineVisible) {
        g.setColour(_outlineColour);
        g.fillRect(0.0f, area.getHeight() - 1.5f, area.getWidth(), 1.5f);
        g.drawRoundedRectangle(area, cornerSize + 2.0f, 3.0f);
    }
}

void CustomTextButtonLF::drawButtonText(juce::Graphics& g, juce::TextButton& button,
                                        bool /*isMouseOverButton*/, bool /*isButtonDown*/) {
    g.setColour(button.findColour(juce::TextButton::textColourOffId));
    g.setFont(juce::Font(juce::FontOptions(16.0f)));
    g.drawText(button.getButtonText(), button.getLocalBounds(),
               juce::Justification::centred, true);
}

//=============================================================================
// CustomTextButton Implementation
//=============================================================================

CustomTextButton::CustomTextButton(const juce::String& buttonName,
                                   ProfilerStyle::Theme theme) : juce::TextButton(buttonName) {
    setTheme(theme);
    setLookAndFeel(&_customLF);
}

CustomTextButton::~CustomTextButton() {
    setLookAndFeel(nullptr);
}

void CustomTextButton::setTheme(ProfilerStyle::Theme theme) {
    switch (theme) {
        case ProfilerStyle::Theme::Darker:
            setColour(juce::TextButton::buttonColourId, ProfilerStyle::Colors::darkestGrey);
            setColour(juce::TextButton::textColourOffId, ProfilerStyle::Colors::white);
            setOutlineVisible(true);
            break;
        case ProfilerStyle::Theme::Dark:
            setColour(juce::TextButton::buttonColourId, ProfilerStyle::Colors::darkerGrey);
            setColour(juce::TextButton::textColourOffId, ProfilerStyle::Colors::white);
            break;
        case ProfilerStyle::Theme::Light:
            setColour(juce::TextButton::buttonColourId, ProfilerStyle::Colors::darkGrey);
            setColour(juce::TextButton::textColourOffId, ProfilerStyle::Colors::white);
            break;
        case ProfilerStyle::Theme::Orange:
            setColour(juce::TextButton::buttonColourId, ProfilerStyle::Colors::orange);
            setColour(juce::TextButton::textColourOffId, ProfilerStyle::Colors::white);
            break;
    }
}

void CustomTextButton::setOutlineVisible(bool shouldShowOutline) {
    _customLF.setOutlineVisible(shouldShowOutline);
    repaint();
}

void CustomTextButton::setOutlineColour(juce::Colour outlineColour) {
    _customLF.setOutlineColour(outlineColour);
    repaint();
}

void CustomTextButton::paint(juce::Graphics& g) {
    juce::TextButton::paint(g);
}

void CustomTextButton::resized() {
    juce::TextButton::resized();
}
