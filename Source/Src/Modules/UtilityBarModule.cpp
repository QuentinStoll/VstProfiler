#include "Modules/UtilityBarModule.h"

UtilityBarModule::UtilityBarModule()
{
	addAndMakeVisible(_profilMenu);
	addAndMakeVisible(_resetButton);
	addAndMakeVisible(_muteSwitch);
	addAndMakeVisible(_eqSwitch);

	_eqSwitch.setToggleState(true);

	_profilMenu.addItem("Default", 1);
	_profilMenu.addItem("Profil 1", 2);
	_profilMenu.addItem("Profil 2", 3);
	_profilMenu.setSelectedId(1);
}

UtilityBarModule::~UtilityBarModule()
{
	_profilMenu.setLookAndFeel(nullptr);
}

void UtilityBarModule::paint(juce::Graphics& g)
{
}

void UtilityBarModule::resized()
{
	auto area = getLocalBounds();
	auto areaWidth = area.getWidth();
	
	auto profilMenuArea = area.removeFromLeft(areaWidth * 0.4f);
	auto resetButtonArea = area.removeFromLeft(areaWidth * 0.2f);
	auto muteSwitchArea = area.removeFromLeft(areaWidth * 0.2f);
	auto eqSwitchArea = area;

	_profilMenu.setBounds(profilMenuArea);
	_resetButton.setBounds(resetButtonArea);
	_muteSwitch.setBounds(muteSwitchArea);
	_eqSwitch.setBounds(eqSwitchArea);
}