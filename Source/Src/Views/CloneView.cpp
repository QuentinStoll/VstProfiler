#include "Views/CloneView.h"

#include "Stylesheet.h"

CloneView::CloneView(ProfilerAudioProcessor& p)
    : _audioProcessor(p) {
    addAndMakeVisible(_sweepButton);
    _sweepButton.onClick = [this]() {
        _audioProcessor.startAmpProfiling();
    };

    addAndMakeVisible(_loadIRButton);
    _loadIRButton.onClick = [this]() {
        _audioProcessor.loadIRFile();
    };

    addAndMakeVisible(_loadAmpButton);
    _loadAmpButton.onClick = [this]() {
        _audioProcessor.startGainAnalysis();
    };
}

CloneView::~CloneView() {
    setLookAndFeel(nullptr);
}

void CloneView::paint(juce::Graphics& g) {
    auto area = getLocalBounds().toFloat();
    juce::Path path;

    path.addRoundedRectangle(area, 5.0f);

    g.setGradientFill(ProfilerStyle::Gradients::vertical(
        area,
        ProfilerStyle::Colors::darkestGrey,
        ProfilerStyle::Colors::darkestGrey.brighter(0.1f),
        0.8f));
    g.fillPath(path);
}

void CloneView::resized() {
    auto area = getLocalBounds().reduced(static_cast<int>(getWidth() * 0.1f));

    float spacing = getWidth() * 0.01f;
    float buttonWidth = (area.getWidth() - (spacing * 2)) / 3.0f;

    float buttonHeight = buttonWidth;

    auto rowArea = area.withHeight(static_cast<int>(buttonHeight)).withCentre(getLocalBounds().getCentre());

    _sweepButton.setBounds(rowArea.removeFromLeft(static_cast<int>(buttonWidth)).toNearestInt());
    rowArea.removeFromLeft(static_cast<int>(spacing));
    _loadIRButton.setBounds(rowArea.removeFromLeft(static_cast<int>(buttonWidth)).toNearestInt());
    rowArea.removeFromLeft(static_cast<int>(spacing));
    _loadAmpButton.setBounds(rowArea.removeFromLeft(static_cast<int>(buttonWidth)).toNearestInt());
}