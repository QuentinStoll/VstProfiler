#include "Modules/BasicEqModule.h"

BasicEqModule::BasicEqModule()
{
	addAndMakeVisible(_bassSlider);
	addAndMakeVisible(_midSlider);
	addAndMakeVisible(_trebleSlider);
	addAndMakeVisible(_presenceSlider);
	addAndMakeVisible(_depthSlider);
}

BasicEqModule::~BasicEqModule()
{
}

void BasicEqModule::paint(juce::Graphics& g)
{
}

void BasicEqModule::resized()
{
	auto area = getLocalBounds();
	auto areaWidth = area.getWidth();

	auto topArea = area.removeFromTop(getHeight() * 0.5f);

	auto bassArea = topArea.removeFromLeft(areaWidth/3);
	auto midArea = topArea.removeFromLeft(areaWidth/3);
	auto trebleArea = topArea;

	area = area.withSizeKeepingCentre(bassArea.getWidth() * 2, area.getHeight());

	auto presenceArea = area.removeFromLeft(bassArea.getWidth());
	auto depthArea = area.removeFromLeft(bassArea.getWidth());

	_bassSlider.setBounds(bassArea);
	_midSlider.setBounds(midArea);
	_trebleSlider.setBounds(trebleArea);
	_presenceSlider.setBounds(presenceArea);
	_depthSlider.setBounds(depthArea);
}