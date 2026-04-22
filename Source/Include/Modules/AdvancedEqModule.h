#include "JuceHeader.h"
#include "Components/CustomSlider.h"

class AdvancedEqModule : public juce::Component
{
public:
	AdvancedEqModule(juce::AudioProcessorValueTreeState& apvts);
	~AdvancedEqModule();

	void paint(juce::Graphics& g) override;
	void resized() override;

private:
	CustomSlider _bassSlider{ "Bass", -24.0f, 24.0f, 0.0f, "dB" };
	CustomSlider _midSlider{ "Mid", -24.0f, 24.0f, 0.0f, "dB" };
	CustomSlider _trebleSlider{ "Treble", -24.0f, 24.0f, 0.0f, "dB" };
	CustomSlider _presenceSlider{ "Presence", -24.0f, 24.0f, 0.0f, "dB" };
	CustomSlider _depthSlider{ "Depth", -24.0f, 24.0f, 0.0f, "dB" };

	using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
	std::unique_ptr<SliderAttachment> _bassAttachment;
	std::unique_ptr<SliderAttachment> _midAttachment;
	std::unique_ptr<SliderAttachment> _trebleAttachment;
	std::unique_ptr<SliderAttachment> _presenceAttachment;
	std::unique_ptr<SliderAttachment> _depthAttachment;

	juce::Component _bandLabelsZone;
};