#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ViewManager.h"

//==============================================================================
/**
*/
class ProfilerAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    ProfilerAudioProcessorEditor (ProfilerAudioProcessor&);
    ~ProfilerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ProfilerAudioProcessor& _audioProcessor;
	
	// Init ViewManager
	std::unique_ptr<ViewManager> _viewManager;

	juce::TextButton SweepButton{ "Start sweep"};
    juce::TextButton IrButton{ "load ir" };
    juce::TextButton AmpButton{ "load amp" };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProfilerAudioProcessorEditor)
};
