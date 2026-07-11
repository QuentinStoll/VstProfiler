#include "Components/CustomKnob.h"
#include "Components/CustomLevelMeter.h"
#include "JuceHeader.h"

class ProfilerAudioProcessor;

class MasterSlidersModule : public juce::Component, private juce::Timer {
   public:
    MasterSlidersModule(ProfilerAudioProcessor& processor);
    ~MasterSlidersModule();

    void paint(juce::Graphics& g) override;
    void resized() override;

   private:
    void timerCallback() override;

    ProfilerAudioProcessor& _audioProcessor;
    CustomKnob _masterVolumeKnob{"Master Volume", 0, 100, 50, "%", 1.0f};
    Gui::VerticalLevelMeter _outputLevelMeter;
    CustomKnob _gainKnob{"Gain", -12.0f, 12.0f, 0.0f, "dB"};
    CustomKnob _noiseGateKnob{"Noise Gate", 0.0f, 60.0f, 10.0f, "dB"};

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> _masterVolumeAttachment;
    std::unique_ptr<SliderAttachment> _gainAttachment;
    std::unique_ptr<SliderAttachment> _noiseGateAttachment;
};
