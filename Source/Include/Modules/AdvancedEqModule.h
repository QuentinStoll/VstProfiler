#include "JuceHeader.h"

class AdvancedEqModule : public juce::Component
{
public:
	AdvancedEqModule();
	~AdvancedEqModule();

	void paint(juce::Graphics& g) override;
	void resized() override;

private:

};