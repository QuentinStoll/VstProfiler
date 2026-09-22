#include "Components/CustomTextButton.h"

CustomTextButton::CustomTextButton(const juce::String& buttonName,
                                   ProfilerStyle::Theme theme) : juce::TextButton(buttonName) {
    setTheme(theme);
}

void CustomTextButton::setTheme(ProfilerStyle::Theme theme) {
    switch (theme) {
        case ProfilerStyle::Theme::Darker:
            setColour(juce::TextButton::buttonColourId, ProfilerStyle::Colors::background);
            setColour(juce::TextButton::textColourOffId, ProfilerStyle::Colors::text);
            setOutlineVisible(true);
            break;
        case ProfilerStyle::Theme::Dark:
            setColour(juce::TextButton::buttonColourId, ProfilerStyle::Colors::elevated);
            setColour(juce::TextButton::textColourOffId, ProfilerStyle::Colors::text);
            break;
        case ProfilerStyle::Theme::Light:
            setColour(juce::TextButton::buttonColourId, ProfilerStyle::Colors::darkGrey);
            setColour(juce::TextButton::textColourOffId, ProfilerStyle::Colors::text);
            break;
        case ProfilerStyle::Theme::Orange:
            setColour(juce::TextButton::buttonColourId, ProfilerStyle::Colors::accent);
            setColour(juce::TextButton::textColourOffId, ProfilerStyle::Colors::text);
            break;
    }
}

void CustomTextButton::setOutlineVisible(bool shouldShowOutline) {
    getProperties().set(ProfilerStyle::Properties::outlineVisible, shouldShowOutline);
    repaint();
}

void CustomTextButton::setOutlineColour(juce::Colour outlineColour) {
    getProperties().set(ProfilerStyle::Properties::outlineColour, static_cast<int>(outlineColour.getARGB()));
    repaint();
}
