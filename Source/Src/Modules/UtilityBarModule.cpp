#include "Modules/UtilityBarModule.h"

UtilityBarModule::UtilityBarModule(juce::AudioProcessorValueTreeState& apvts) : _apvts(apvts) {
    addAndMakeVisible(_profilMenu);
    addAndMakeVisible(_resetButton);
    addAndMakeVisible(_muteSwitch);
    addAndMakeVisible(_eqSwitch);

    _eqSwitch.setToggleState(true, false);

    _profilMenu.addItem("Default", 1);
    _profilMenu.addItem("Profil 1", 2);
    _profilMenu.addItem("Profil 2", 3);
    _profilMenu.setSelectedId(1);

    _resetButton.onClick = [this]() {
        resetAllParameters();
    };
}

UtilityBarModule::~UtilityBarModule() {
    _profilMenu.setLookAndFeel(nullptr);
}

void UtilityBarModule::paint(juce::Graphics& g) {
}

void UtilityBarModule::resized() {
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

void UtilityBarModule::resetAllParameters() {
    auto resetParam = [this](const juce::String& paramID) {
        auto* param = _apvts.getParameter(paramID);
        if (param != nullptr)
            param->setValueNotifyingHost(param->getDefaultValue());
    };

    resetParam("master");
    resetParam("gain");
    resetParam("noise");
    resetParam("bass");
    resetParam("depth");
    resetParam("mid");
    resetParam("presence");
    resetParam("treble");
}