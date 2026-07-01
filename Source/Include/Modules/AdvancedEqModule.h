#include "Components/CustomKnob.h"
#include "JuceHeader.h"

class AdvancedEqModule : public juce::Component {
   public:
    AdvancedEqModule(juce::AudioProcessorValueTreeState& apvts);
    ~AdvancedEqModule();

    void paint(juce::Graphics& g) override;
    void resized() override;

   private:
    CustomKnob _bassKnob{"Bass", -24.0f, 24.0f, 0.0f, "dB"};
    CustomKnob _midKnob{"Mid", -24.0f, 24.0f, 0.0f, "dB"};
    CustomKnob _trebleKnob{"Treble", -24.0f, 24.0f, 0.0f, "dB"};
    CustomKnob _presenceKnob{"Presence", -24.0f, 24.0f, 0.0f, "dB"};
    CustomKnob _depthKnob{"Depth", -24.0f, 24.0f, 0.0f, "dB"};

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> _bassAttachment;
    std::unique_ptr<SliderAttachment> _midAttachment;
    std::unique_ptr<SliderAttachment> _trebleAttachment;
    std::unique_ptr<SliderAttachment> _presenceAttachment;
    std::unique_ptr<SliderAttachment> _depthAttachment;

    juce::Component _bandLabelsZone;
};