#pragma once

#include <JuceHeader.h>

#include "Modules/FileAssetsModule.h"

class ProfilerAudioProcessor;

class CloneView : public juce::Component {
   public:
    CloneView(ProfilerAudioProcessor& p);
    ~CloneView();

    void paint(juce::Graphics& g) override;
    void resized() override;

   private:
    FileAssetsModule _fileAssetsModule;
};
