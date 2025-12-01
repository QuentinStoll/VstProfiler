/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ProfilerAudioProcessorEditor::ProfilerAudioProcessorEditor (ProfilerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
	viewManager = std::make_unique<ViewManager>();
	addAndMakeVisible(viewManager.get());
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (viewManager->getWidth(), viewManager->getHeight());
}

ProfilerAudioProcessorEditor::~ProfilerAudioProcessorEditor()
{
}

//==============================================================================
void ProfilerAudioProcessorEditor::paint (juce::Graphics& g)
{
    //// (Our component is opaque, so we must completely fill the background with a solid colour)
    //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    //g.setColour (juce::Colours::white);
    //g.setFont (juce::FontOptions (15.0f));
    //g.drawFittedText ("Neg!", getLocalBounds(), juce::Justification::centred, 1);
}

void ProfilerAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    viewManager->setBounds(getLocalBounds());
}
