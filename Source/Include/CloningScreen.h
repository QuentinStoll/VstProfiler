#pragma once
#include <JuceHeader.h>

#include "PluginProcessor.h"

class CloningScreen : public juce::Component {
   public:
    CloningScreen(ProfilerAudioProcessor& p);
    ~CloningScreen();

    void paint(juce::Graphics& g) override;
    void resized() override;

   private:
    ProfilerAudioProcessor& _audioProcessor;

    juce::TextButton _sweepButton{"Start sweep"};
    juce::TextButton _irButton{"load ir"};
    juce::TextButton _ampButton{"load amp"};
    juce::TextButton _neuralButton{"load neural"};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CloningScreen)
};