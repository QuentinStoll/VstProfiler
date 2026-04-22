#include "Modules/MasterSlidersModule.h"

MasterSlidersModule::MasterSlidersModule(juce::AudioProcessorValueTreeState& apvts)
{
	addAndMakeVisible(_masterVolumeSlider);
	addAndMakeVisible(_gainSlider);
	addAndMakeVisible(_noiseGateSlider);

	_masterVolumeAttachment = std::make_unique<SliderAttachment>(apvts, "master", _masterVolumeSlider.getSlider());
	_gainAttachment = std::make_unique<SliderAttachment>(apvts, "gain", _gainSlider.getSlider());
	_noiseGateAttachment = std::make_unique<SliderAttachment>(apvts, "noise", _noiseGateSlider.getSlider());
}

MasterSlidersModule::~MasterSlidersModule()
{
}

void MasterSlidersModule::paint(juce::Graphics& g)
{
}

void MasterSlidersModule::resized()
{
	auto area = getLocalBounds();
	auto areaWidth = area.getWidth();

	auto sideArea = area.removeFromLeft(areaWidth/3);
	auto noiseGateArea = sideArea.removeFromTop(getHeight() * 0.5f);
	auto gainArea = sideArea;

	_masterVolumeSlider.setBounds(area);
	_noiseGateSlider.setBounds(noiseGateArea);
	_gainSlider.setBounds(gainArea);
}