#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Components/CustomTabs.h"

//==============================================================================
class ProfilerAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    ProfilerAudioProcessorEditor (ProfilerAudioProcessor&);
    ~ProfilerAudioProcessorEditor() override;

    //
    void paint (juce::Graphics&) override;
    
    //
    void resized() override;

private:
    
    //
    ProfilerAudioProcessor& _audioProcessor;

    //
    CustomTabs _tabs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProfilerAudioProcessorEditor)
};
