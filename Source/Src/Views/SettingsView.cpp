#include "Views/SettingsView.h"

#include "Stylesheet.h"
#include "UiSettings.h"

namespace {
enum BackgroundPresetId {
    defaultPreset = 1,
    bluePreset,
    purplePreset,
    greenPreset,
    redPreset
};

void repaintComponentTree(juce::Component& component) {
    component.repaint();

    for (int index = 0; index < component.getNumChildComponents(); ++index) {
        if (auto* child = component.getChildComponent(index)) {
            repaintComponentTree(*child);
        }
    }
}

void applyBackgroundColourPreset(int presetId) {
    switch (UiSettings::normaliseBackgroundColourPreset(presetId)) {
        case bluePreset:
            ProfilerStyle::Colors::appBackground = juce::Colour(0xff182a44);
            break;

        case purplePreset:
            ProfilerStyle::Colors::appBackground = juce::Colour(0xff30203f);
            break;

        case greenPreset:
            ProfilerStyle::Colors::appBackground = juce::Colour(0xff18352a);
            break;

        case redPreset:
            ProfilerStyle::Colors::appBackground = juce::Colour(0xff431f1f);
            break;

        case defaultPreset:
        default:
            ProfilerStyle::Colors::appBackground = ProfilerStyle::Colors::darkestGrey;
            break;
    }
}
}  // namespace

SettingsView::SettingsView() {
    _titleLabel.setText("Settings", juce::dontSendNotification);
    _titleLabel.setJustificationType(juce::Justification::centred);
    _titleLabel.setFont(juce::FontOptions(24.0f, juce::Font::bold));
    _titleLabel.setColour(juce::Label::textColourId, ProfilerStyle::Colors::white);

    addAndMakeVisible(_titleLabel);

    _backgroundLabel.setText("Background colour", juce::dontSendNotification);
    _backgroundLabel.setColour(juce::Label::textColourId, ProfilerStyle::Colors::white);

    addAndMakeVisible(_backgroundLabel);

    _backgroundMenu.addItem("Default", defaultPreset);
    _backgroundMenu.addItem("Blue", bluePreset);
    _backgroundMenu.addItem("Purple", purplePreset);
    _backgroundMenu.addItem("Green", greenPreset);
    _backgroundMenu.addItem("Red", redPreset);

    _backgroundMenu.setSelectedId(UiSettings::loadBackgroundColourPreset(),
                                  juce::dontSendNotification);
    applyBackgroundColour();

    _backgroundMenu.onChange = [this]() {
        applyBackgroundColour();
    };

    addAndMakeVisible(_backgroundMenu);
}

void SettingsView::applySavedBackgroundColour() {
    applyBackgroundColourPreset(UiSettings::loadBackgroundColourPreset());
}

void SettingsView::paint(juce::Graphics& g) {
    const auto area = getLocalBounds().toFloat();

    g.setGradientFill(ProfilerStyle::Gradients::vertical(
        area,
        ProfilerStyle::Colors::appBackground,
        ProfilerStyle::Colors::appBackground.brighter(0.1f),
        0.8f));

    g.fillRoundedRectangle(area, 5.0f);
}

void SettingsView::resized() {
    auto area = getLocalBounds().reduced(40);

    _titleLabel.setBounds(area.removeFromTop(50));

    area.removeFromTop(30);

    auto settingLine = area.removeFromTop(40);

    _backgroundLabel.setBounds(settingLine.removeFromLeft(180));

    settingLine.removeFromLeft(20);

    _backgroundMenu.setBounds(settingLine.removeFromLeft(220));
}

void SettingsView::applyBackgroundColour() {
    applyBackgroundColourPreset(_backgroundMenu.getSelectedId());
    UiSettings::saveBackgroundColourPreset(_backgroundMenu.getSelectedId());
    refreshColours();
}

void SettingsView::refreshColours() {
    if (auto* editor = getTopLevelComponent()) {
        repaintComponentTree(*editor);
    }
}
