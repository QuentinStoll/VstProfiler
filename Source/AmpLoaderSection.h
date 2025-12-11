/*
  ==============================================================================

    AmpLoaderSection.h
    Created: 2 Dec 2025 3:49:03pm
    Author:  Laurent ZHANG

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class AmpLoaderSection : public juce::Component
{
public:
	AmpLoaderSection();
	~AmpLoaderSection();

	void paint(juce::Graphics& g) override;
	void resized() override;

private:

};

