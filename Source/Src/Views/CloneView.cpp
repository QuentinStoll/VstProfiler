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
    auto area = getLocalBounds().reduced(getWidth() * 0.1f);

    float spacing = getWidth() * 0.01f;
    float buttonWidth = (area.getWidth() - (spacing * 2)) / 3.0f;

    float buttonHeight = buttonWidth;

    auto rowArea = area.withHeight(buttonHeight).withCentre(getLocalBounds().getCentre());

    _sweepButton.setBounds(rowArea.removeFromLeft(buttonWidth).toNearestInt());
    rowArea.removeFromLeft(spacing);
    _loadIRButton.setBounds(rowArea.removeFromLeft(buttonWidth).toNearestInt());
    rowArea.removeFromLeft(spacing);
    _loadAmpButton.setBounds(rowArea.removeFromLeft(buttonWidth).toNearestInt());
}