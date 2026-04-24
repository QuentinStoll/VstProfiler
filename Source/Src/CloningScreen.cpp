#include "CloningScreen.h"

CloningScreen::CloningScreen(ProfilerAudioProcessor& p) : _audioProcessor(p) {
    addAndMakeVisible(_sweepButton);
    _sweepButton.onClick = [this]() { _audioProcessor.startAmpProfiling(); };

    addAndMakeVisible(_irButton);
    _irButton.onClick = [this]() { _audioProcessor.loadIRFile(); };

    addAndMakeVisible(_ampButton);
    _ampButton.onClick = [this]() { _audioProcessor.startGainAnalysis(); };

    addAndMakeVisible(_neuralButton);
    _neuralButton.onClick = [this]() {
        auto chooser = new juce::FileChooser("Select a neural model file", {}, "*.onnx");
        chooser->launchAsync(juce::FileBrowserComponent::openMode |
                                 juce::FileBrowserComponent::canSelectFiles,
                             [this, chooser](const juce::FileChooser& fc) {
                                 auto file = fc.getResult();
                                 if (file.existsAsFile()) {
                                     _audioProcessor.loadNeuralModel(file);
                                 }
                                 delete chooser;  // clean up memory
                             });
    };
    

    setSize(600, 400);
}

CloningScreen::~CloningScreen() {}

void CloningScreen::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::darkgrey);
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(20.0f));
    g.drawFittedText("Entering Cloning mode", getLocalBounds(),
                     juce::Justification::centredTop, 1);
}

void CloningScreen::resized() {
    auto bounds = getLocalBounds().reduced(10);
    auto buttonWidth = bounds.getWidth() / 2 - 20;
    auto buttonHeight = 40;

    _sweepButton.setBounds(bounds.getX(), 100, getWidth() - 20, 30);
    _irButton.setBounds(bounds.getX(), 150, getWidth() - 20, 30);
    _ampButton.setBounds(bounds.getX(), 190, getWidth() - 20, 30);
    _neuralButton.setBounds(bounds.getX(), 230, getWidth() - 20, 30);
    //_startCloneButton.setBounds(bounds.getCentreX() - (buttonWidth / 2),
    // bounds.getCentreY(), buttonWidth,
    // buttonHeight);
}