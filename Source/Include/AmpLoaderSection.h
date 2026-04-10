#pragma once
#include <JuceHeader.h>

class AmpLoaderSection : public juce::Component {
   public:
    AmpLoaderSection();
    ~AmpLoaderSection();

    void paint(juce::Graphics& g) override;
    void resized() override;

   private:
};
