#pragma once

#include <array>
#include <memory>

#include "Components/CustomKnob.h"
#include "EqBandLayout.h"
#include "JuceHeader.h"

class AdvancedEqModule : public juce::Component {
   public:
    explicit AdvancedEqModule(juce::AudioProcessorValueTreeState& apvts);
    ~AdvancedEqModule() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

   private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::array<std::unique_ptr<CustomKnob>, EqBands::count> _gainKnobs;
    std::array<std::unique_ptr<CustomKnob>, EqBands::count> _freqKnobs;
    std::array<std::unique_ptr<SliderAttachment>, EqBands::count> _gainAttachments;
    std::array<std::unique_ptr<SliderAttachment>, EqBands::count> _freqAttachments;
};
