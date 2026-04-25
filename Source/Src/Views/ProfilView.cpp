#include "Views/ProfilView.h"

#include "Styles/Stylesheet.h"

ProfilView::ProfilView(ProfilerAudioProcessor& p)
    : _audioProcessor(p) {
    addAndMakeVisible(_sampleButton);
    _sampleButton.setButtonText("Sample Button Profil");
}

ProfilView::~ProfilView() {
}

void ProfilView::paint(juce::Graphics& g) {
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

void ProfilView::resized() {
    _sampleButton.setBounds(20, 20, 200, 30);
}