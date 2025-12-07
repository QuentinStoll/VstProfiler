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
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
    
	SweepButton.setButtonText("Start Sweep");
	addAndMakeVisible(SweepButton);
    SweepButton.onClick = [this]()
    {
        audioProcessor.startSweep();
    };

    IrButton.setButtonText("Load Ir");
    addAndMakeVisible(IrButton);
    IrButton.onClick = [this]()
    {
        audioProcessor.loadIRFile();
    };

    AmpButton.setButtonText("Load Amp");
    addAndMakeVisible(AmpButton);
    AmpButton.onClick = [this]()
    {
        audioProcessor.loadAmpProfile();
    };


}

ProfilerAudioProcessorEditor::~ProfilerAudioProcessorEditor()
{
}

//==============================================================================
void ProfilerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    //g.drawFittedText ("Neg!", getLocalBounds(), juce::Justi
    // fication::centred, 1);
}

void ProfilerAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

	SweepButton.setBounds(10, 10, getWidth() - 20, 30);
    IrButton.setBounds(10, 50, getWidth() - 20, 30);
    AmpButton.setBounds(10, 90, getWidth() - 20, 30);
}

