#pragma once

#include <JuceHeader.h>

#include "AmpEngine.h"

class AmpProfiling : public juce::Component {
   public:
    void generateAndSaveGainSignal();
    void startGainAnalysis(AmpProcessor& processor);
    void processGainAnalysis(juce::File fileRef, juce::File fileRec, AmpProcessor& processor);

    void generateSaturationProbe();
    void startSaturationAnalysis(AmpProcessor& processor);
    void saveAmpConfig(const juce::File& file);

   private:
    std::unique_ptr<juce::FileChooser> _chooser;
};