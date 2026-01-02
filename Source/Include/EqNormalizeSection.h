/*
  ==============================================================================

    EqNormalizeSection.h
    Created: 2 Dec 2025 3:48:38pm
    Author:  Laurent ZHANG

  ==============================================================================
*/

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
