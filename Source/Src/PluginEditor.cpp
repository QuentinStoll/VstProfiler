#include "PluginEditor.h"

#include "PluginProcessor.h"

//==============================================================================
ProfilerAudioProcessorEditor::ProfilerAudioProcessorEditor(
    ProfilerAudioProcessor& p)
    : AudioProcessorEditor(&p), _audioProcessor(p) {
    _viewManager = std::make_unique<ViewManager>(p);
    addAndMakeVisible(_viewManager.get());
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(_viewManager->getWidth(), _viewManager->getHeight());
}

ProfilerAudioProcessorEditor::~ProfilerAudioProcessorEditor() {}

//==============================================================================
void ProfilerAudioProcessorEditor::paint(juce::Graphics& g) {}

void ProfilerAudioProcessorEditor::resized() {
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    _viewManager->setBounds(getLocalBounds());
}
