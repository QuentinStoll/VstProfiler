#pragma once

#include <JuceHeader.h>

#include "Modules/FileAssetsModule.h"

class ProfilerAudioProcessor;

class CloneView : public juce::Component,
                  private juce::ChangeListener {
   public:
    CloneView(ProfilerAudioProcessor& p);
    ~CloneView();

    void paint(juce::Graphics& g) override;
    void resized() override;

   private:
    ProfilerAudioProcessor& _audioProcessor;
    FileAssetsModule _fileAssetsModule;

    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
};
