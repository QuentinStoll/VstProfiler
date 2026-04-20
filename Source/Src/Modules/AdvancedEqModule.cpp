#include "Modules/AdvancedEqModule.h"

AdvancedEqModule::AdvancedEqModule()
{
	addAndMakeVisible(_bassSlider);
	addAndMakeVisible(_midSlider);
	addAndMakeVisible(_trebleSlider);
	addAndMakeVisible(_presenceSlider);
	addAndMakeVisible(_depthSlider);

	addAndMakeVisible(_bandLabelsZone);
}

AdvancedEqModule::~AdvancedEqModule()
{
}

void AdvancedEqModule::paint(juce::Graphics& g)
{
	g.setColour(juce::Colours::blue.withAlpha(0.2f));
	g.fillRect(_bandLabelsZone.getBounds());
}

void AdvancedEqModule::resized()
{
	auto area = getLocalBounds();
	auto areaWidth = area.getWidth();

	auto topArea = area.removeFromTop(getHeight() * 0.4f);

	auto bassArea = topArea.removeFromLeft(areaWidth / 5);
	auto midArea = topArea.removeFromLeft(areaWidth / 5);
	auto trebleArea = topArea.removeFromLeft(areaWidth / 5);
	auto presenceArea = topArea.removeFromLeft(areaWidth / 5);
	auto depthArea = topArea;

	_bassSlider.setBounds(bassArea);
	_midSlider.setBounds(midArea);
	_trebleSlider.setBounds(trebleArea);
	_presenceSlider.setBounds(presenceArea);
	_depthSlider.setBounds(depthArea);

	_bandLabelsZone.setBounds(area);
}