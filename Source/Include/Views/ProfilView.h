#include "JuceHeader.h"
#include "PluginProcessor.h"

class ProfilView : public juce::Component
{
public:
	ProfilView(ProfilerAudioProcessor& p);
	~ProfilView();

	void paint(juce::Graphics& g) override;
	void resized() override;

private:
	juce::TextButton _sampleButton{ "Sample Button" };
	ProfilerAudioProcessor& _audioProcessor;
};