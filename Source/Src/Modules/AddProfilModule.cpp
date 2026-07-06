#include "Modules/AddProfilModule.h"

#include "BinaryData.h"
#include "Stylesheet.h"

AddProfilModule::AddProfilModule()
    : _createProfilButton("Create profil", BinaryData::plusicon_svg, BinaryData::plusicon_svgSize, ProfilerStyle::Theme::Darker),
      _importProfilButton("Import profil", BinaryData::plusicon_svg, BinaryData::plusicon_svgSize, ProfilerStyle::Theme::Darker) {
    addAndMakeVisible(_title);
    addAndMakeVisible(_createProfilButton);
    addAndMakeVisible(_importProfilButton);

    _title.setText("Adding a profil", juce::dontSendNotification);
    _title.setJustificationType(juce::Justification::centred);
    _title.setColour(juce::Label::textColourId, ProfilerStyle::Colors::white);
    _title.setFont(juce::Font(juce::FontOptions(18.0f)));

    _createProfilButton.onClick = [this]() {
        createProfilClicked();
    };

    _importProfilButton.onClick = [this]() {
        importProfilClicked();
    };
}

void AddProfilModule::paint(juce::Graphics& /*g*/) {
}

void AddProfilModule::resized() {
    auto area = getLocalBounds().reduced(20);

    _title.setBounds(area.removeFromTop(28));
    area.removeFromTop(20);

    const auto gap = 40;
    const auto buttonWidth = (area.getWidth() - gap) / 2;
    const auto buttonHeight = juce::jmin(area.getHeight(), buttonWidth);
    auto buttonArea = area.withSizeKeepingCentre(area.getWidth(), buttonHeight);

    _createProfilButton.setBounds(buttonArea.removeFromLeft(buttonWidth));
    buttonArea.removeFromLeft(gap);
    _importProfilButton.setBounds(buttonArea.removeFromLeft(buttonWidth));
}

void AddProfilModule::createProfilClicked() {
    if (onCreateProfilClicked)
        onCreateProfilClicked();
}

void AddProfilModule::importProfilClicked() {
    if (onImportProfilClicked)
        onImportProfilClicked();
}
