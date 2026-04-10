#include "JuceHeader.h"
#include "Components/CustomSlider.h"

class MasterSlidersModule : public juce::Component
{
public:
	MasterSlidersModule();
	~MasterSlidersModule();

	void paint(juce::Graphics& g) override;
	void resized() override;

private:
	CustomSlider _masterVolumeSlider{ "Master Volume", 0, 100, 50, "%", 1.0f};
	CustomSlider _gainSlider{ "Gain", -12.0f, 12.0f, 0.0f, "dB" }; 
	CustomSlider _noiseGateSlider{ "Noise", 0.0f, 60.0f, 10.0f, "dB" };
};