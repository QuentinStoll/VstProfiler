#pragma once

#include <array>
#include <memory>

#include "Components/CustomKnob.h"
#include "EqBandLayout.h"
#include "JuceHeader.h"

class BasicEqModule : public juce::Component {
   public:
    explicit BasicEqModule(juce::AudioProcessorValueTreeState& apvts);
    ~BasicEqModule() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

   private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::array<std::unique_ptr<CustomKnob>, EqBands::count> _knobs;
    std::array<std::unique_ptr<SliderAttachment>, EqBands::count> _attachments;
};
