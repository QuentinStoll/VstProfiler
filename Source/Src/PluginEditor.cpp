#include "PluginEditor.h"

#include "PluginProcessor.h"
#include "ProfilerConstantValues.h"
#include "Stylesheet.h"
#include "Views/CloneView.h"
#include "Views/PlayView.h"
#include "Views/ProfilView.h"
#include "Views/SettingsView.h"

//==============================================================================
ProfilerAudioProcessorEditor::ProfilerAudioProcessorEditor(ProfilerAudioProcessor& p)
    : AudioProcessorEditor(&p),
      _audioProcessor(p),
      _tabs({{"Clone", [&p] { return std::make_unique<CloneView>(p); }},
             {"Play", [&p] { return std::make_unique<PlayView>(p); }},
             {"Profil", [&p] { return std::make_unique<ProfilView>(p); }},
             {"Settings", [] { return std::make_unique<SettingsView>(); }}},
            1) {
    SettingsView::applySavedBackgroundColour();

    // Define the initial size of the plugin window
    setResizable(true, true);
    getConstrainer()->setFixedAspectRatio(windowHeight / (double)windowWidth);

    // Set the limits of the plugin window resizing
    setResizeLimits(windowHeight, windowWidth, windowHeight * 3, windowWidth * 3);
    setSize(windowHeight, windowWidth);

    addAndMakeVisible(_tabs);
}

//==============================================================================
ProfilerAudioProcessorEditor::~ProfilerAudioProcessorEditor() {
}

//==============================================================================
void ProfilerAudioProcessorEditor::paint(juce::Graphics& g) {
    auto area = getLocalBounds().toFloat();

    // Set the background color of the plugin
    g.setGradientFill(ProfilerStyle::Gradients::vertical(
        area,
        ProfilerStyle::Colors::appBackground.brighter(0.2f),
        ProfilerStyle::Colors::appBackground.darker(0.4f),
        0.2f));

    g.fillAll();
}

//==============================================================================
void ProfilerAudioProcessorEditor::resized() {
    auto area = getLocalBounds();

    _tabs.setBounds(area.reduced(5));
}
