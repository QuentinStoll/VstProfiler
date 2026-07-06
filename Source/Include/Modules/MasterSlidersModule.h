#include "Components/CustomKnob.h"
#include "JuceHeader.h"

class MasterSlidersModule : public juce::Component {
   public:
    MasterSlidersModule(juce::AudioProcessorValueTreeState& apvts);
    ~MasterSlidersModule();

    void paint(juce::Graphics& g) override;
    void resized() override;

   private:
    CustomKnob _masterVolumeKnob{"Master Volume", 0, 100, 50, "%", 1.0f};
    CustomKnob _gainKnob{"Gain", -12.0f, 12.0f, 0.0f, "dB"};
    CustomKnob _noiseGateKnob{"Noise Gate", 0.0f, 60.0f, 10.0f, "dB"};

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> _masterVolumeAttachment;
    std::unique_ptr<SliderAttachment> _gainAttachment;
    std::unique_ptr<SliderAttachment> _noiseGateAttachment;
};