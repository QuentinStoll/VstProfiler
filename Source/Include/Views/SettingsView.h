#pragma once

#include <JuceHeader.h>
#include "Components/CustomComboBox.h"

class SettingsView : public juce::Component {
public:
    SettingsView();

    static void applySavedBackgroundColour();

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::Label _titleLabel;
    juce::Label _backgroundLabel;
    CustomComboBox _backgroundMenu;

    void applyBackgroundColour();
    void refreshColours();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsView)
};
