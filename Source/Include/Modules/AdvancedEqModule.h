#include "JuceHeader.h"
#include "Components/CustomSlider.h"

class AdvancedEqModule : public juce::Component
{
public:
	AdvancedEqModule();
	~AdvancedEqModule();

	void paint(juce::Graphics& g) override;
	void resized() override;

private:
	CustomSlider _bassSlider{ "Bass", -24.0f, 24.0f, 0.0f, "dB" };
	CustomSlider _midSlider{ "Mid", -24.0f, 24.0f, 0.0f, "dB" };
	CustomSlider _trebleSlider{ "Treble", -24.0f, 24.0f, 0.0f, "dB" };
	CustomSlider _presenceSlider{ "Presence", -24.0f, 24.0f, 0.0f, "dB" };
	CustomSlider _depthSlider{ "Depth", -24.0f, 24.0f, 0.0f, "dB" };

	juce::Component _bandLabelsZone;
};