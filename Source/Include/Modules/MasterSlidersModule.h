#include "Components/CustomSlider.h"
#include "JuceHeader.h"

class MasterSlidersModule : public juce::Component {
   public:
    MasterSlidersModule(juce::AudioProcessorValueTreeState& apvts);
    ~MasterSlidersModule();

    void paint(juce::Graphics& g) override;
    void resized() override;

   private:
    CustomSlider _masterVolumeSlider{"Master Volume", 0, 100, 50, "%", 1.0f};
    CustomSlider _gainSlider{"Gain", -12.0f, 12.0f, 0.0f, "dB"};
    CustomSlider _noiseGateSlider{"Noise Gate", 0.0f, 60.0f, 10.0f, "dB"};

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> _masterVolumeAttachment;
    std::unique_ptr<SliderAttachment> _gainAttachment;
    std::unique_ptr<SliderAttachment> _noiseGateAttachment;
};