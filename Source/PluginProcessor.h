/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class ProfilerAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    ProfilerAudioProcessor();
    ~ProfilerAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

	//==============================================================================
    juce::AudioProcessorValueTreeState _apvts{ *this, nullptr, "Parameters", createParameterLayout() };

	//==============================================================================
	void updateFilterCoefficients();

private:
    juce::dsp::ProcessorChain <
		juce::dsp::Gain<float>,             // Input Gain
		juce::dsp::NoiseGate<float>,        // Noise Gate
		juce::dsp::IIR::Filter<float>,      // Bass - Low Shelf
		juce::dsp::IIR::Filter<float>,      // Mid - Peak Filter
		juce::dsp::IIR::Filter<float>,      // Treble - High Shelf
		juce::dsp::Gain<float>              // Output Gain
    > _mainProcessor;

    //==============================================================================
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProfilerAudioProcessor)
};
