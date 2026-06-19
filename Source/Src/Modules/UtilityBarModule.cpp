#include "Modules/UtilityBarModule.h"

UtilityBarModule::UtilityBarModule(juce::AudioProcessorValueTreeState& apvts) : _apvts(apvts) {
    addAndMakeVisible(_profilMenu);
    addAndMakeVisible(_resetButton);
    addAndMakeVisible(_muteSwitch);
    addAndMakeVisible(_eqSwitch);

    _eqSwitch.setToggleState(true, juce::dontSendNotification);
    _muteAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(_apvts, "isMute", _muteSwitch);
    _eqAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(_apvts, "isEqEnabled", _eqSwitch);

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

void UtilityBarModule::paint(juce::Graphics& /*g*/) {}

void UtilityBarModule::resized() {
    auto area = getLocalBounds();
    auto areaWidth = area.getWidth();

    auto profilMenuArea = area.removeFromLeft(static_cast<int>(areaWidth * 0.5f));
    area.removeFromLeft(static_cast<int>(areaWidth * 0.01f));
    auto resetButtonArea = area.removeFromLeft(static_cast<int>(areaWidth * 0.2f));
    area.removeFromLeft(static_cast<int>(areaWidth * 0.01f));
    auto muteSwitchArea = area.removeFromLeft(static_cast<int>(areaWidth * 0.1f));
    area.removeFromLeft(static_cast<int>(areaWidth * 0.01f));
    auto eqSwitchArea = area.removeFromLeft(static_cast<int>(areaWidth * 0.1f));

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