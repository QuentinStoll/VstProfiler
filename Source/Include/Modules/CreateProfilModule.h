#pragma once

#include <JuceHeader.h>

class CreateProfilModule : public juce::Component {
   public:
    CreateProfilModule() = default;
    ~CreateProfilModule() override = default;

    void paint(juce::Graphics& g) override;

   private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CreateProfilModule)
};
