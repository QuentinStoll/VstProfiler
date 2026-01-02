#pragma once
#include <JuceHeader.h>

class EqNormalizeSection : public juce::Component
{
public:
	EqNormalizeSection();
	~EqNormalizeSection();

	void paint(juce::Graphics& g) override;
	void resized() override;

private:

};
