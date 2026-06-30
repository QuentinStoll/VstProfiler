#include "Modules/UtilityBarModule.h"

#include "PluginProcessor.h"

UtilityBarModule::UtilityBarModule(ProfilerAudioProcessor& processor)
    : _audioProcessor(processor),
      _apvts(processor._apvts) {
    addAndMakeVisible(_profilMenu);
    addAndMakeVisible(_resetButton);
    addAndMakeVisible(_exportButton);
    addAndMakeVisible(_muteSwitch);
    addAndMakeVisible(_eqSwitch);

    _eqSwitch.setToggleState(true, juce::dontSendNotification);
    _muteAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(_apvts, "isMute", _muteSwitch);
    _eqAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(_apvts, "isEqEnabled", _eqSwitch);

    _audioProcessor.getProfileManager().addChangeListener(this);
    refreshProfileMenu();
    restoreLastUsedProfile();

    _profilMenu.onChange = [this]() {
        selectProfileFromMenu();
    };

    _resetButton.onClick = [this]() {
        resetAllParameters();
    };

    _exportButton.onClick = [this]() {
        if (onExportClicked) {
            onExportClicked();
        }
    };
}

UtilityBarModule::~UtilityBarModule() {
    _audioProcessor.getProfileManager().removeChangeListener(this);
    _profilMenu.setLookAndFeel(nullptr);
}

void UtilityBarModule::paint(juce::Graphics& /*g*/) {}

void UtilityBarModule::resized() {
    auto area = getLocalBounds();
    auto areaWidth = area.getWidth();
    const auto gap = juce::jmax(4, static_cast<int>(areaWidth * 0.01f));

    auto profilMenuArea = area.removeFromLeft(static_cast<int>(areaWidth * 0.42f));
    area.removeFromLeft(juce::jmin(gap, area.getWidth()));
    auto resetButtonArea = area.removeFromLeft(static_cast<int>(areaWidth * 0.15f));
    area.removeFromLeft(juce::jmin(gap, area.getWidth()));
    auto exportButtonArea = area.removeFromLeft(static_cast<int>(areaWidth * 0.15f));
    area.removeFromLeft(juce::jmin(gap, area.getWidth()));
    auto muteSwitchArea = area.removeFromLeft(static_cast<int>(areaWidth * 0.1f));
    area.removeFromLeft(juce::jmin(gap, area.getWidth()));
    auto eqSwitchArea = area;

    _profilMenu.setBounds(profilMenuArea);
    _resetButton.setBounds(resetButtonArea);
    _exportButton.setBounds(exportButtonArea);
    _muteSwitch.setBounds(muteSwitchArea);
    _eqSwitch.setBounds(eqSwitchArea);
}

void UtilityBarModule::resetAllParameters() {
    auto& profileManager = _audioProcessor.getProfileManager();
    const auto currentProfileIndex = profileManager.getCurrentProfileIndex();

    if (currentProfileIndex >= 0) {
        juce::String errorMessage;
        if (_audioProcessor.applyProfile(currentProfileIndex, &errorMessage)) {
            return;
        }
    }

    if (profileManager.getCurrentProfileId().isNotEmpty()) {
        profileManager.clearCurrentProfile();
        refreshProfileMenu();
    }

    resetParametersToDefaults();
}

void UtilityBarModule::resetParametersToDefaults() {
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
    resetParam("isMute");
    resetParam("isEqEnabled");
}

void UtilityBarModule::refreshProfileMenu() {
    const juce::ScopedValueSetter<bool> updatingProfileMenu(_isUpdatingProfileMenu, true);
    auto& profileManager = _audioProcessor.getProfileManager();
    const auto profileNames = profileManager.getProfileNames();

    _profilMenu.clear(juce::dontSendNotification);
    _profilMenu.addItem("Default", 1);

    for (int index = 0; index < profileNames.size(); ++index) {
        const auto profileName = profileNames[index].isNotEmpty()
                                     ? profileNames[index]
                                     : "Profil " + juce::String(index + 1);
        _profilMenu.addItem(profileName, index + 2);
    }

    const auto currentProfileIndex = profileManager.getCurrentProfileIndex();
    const auto selectedId = currentProfileIndex >= 0 ? currentProfileIndex + 2 : 1;
    _profilMenu.setSelectedId(selectedId, juce::dontSendNotification);
}

void UtilityBarModule::restoreLastUsedProfile() {
    auto& profileManager = _audioProcessor.getProfileManager();

    if (profileManager.getCurrentProfileId().isEmpty()) {
        return;
    }

    const auto currentProfileIndex = profileManager.getCurrentProfileIndex();
    if (currentProfileIndex < 0) {
        profileManager.clearCurrentProfile();
        resetParametersToDefaults();
        refreshProfileMenu();
        return;
    }

    juce::String errorMessage;
    if (_audioProcessor.applyProfile(currentProfileIndex, &errorMessage)) {
        const juce::ScopedValueSetter<bool> updatingProfileMenu(_isUpdatingProfileMenu, true);
        _profilMenu.setSelectedId(currentProfileIndex + 2, juce::dontSendNotification);
    } else {
        profileManager.clearCurrentProfile();
        resetParametersToDefaults();
        refreshProfileMenu();
    }
}

void UtilityBarModule::selectProfileFromMenu() {
    if (_isUpdatingProfileMenu) {
        return;
    }

    auto& profileManager = _audioProcessor.getProfileManager();
    const auto selectedId = _profilMenu.getSelectedId();
    if (selectedId == 1) {
        profileManager.clearCurrentProfile();
        resetParametersToDefaults();
        return;
    }

    const auto profileIndex = selectedId - 2;
    juce::String errorMessage;
    if (_audioProcessor.applyProfile(profileIndex, &errorMessage)) {
        if (const auto* profile = profileManager.getProfile(profileIndex)) {
            profileManager.setCurrentProfileId(profile->id);
        }
    } else {
        refreshProfileMenu();
    }
}

void UtilityBarModule::changeListenerCallback(juce::ChangeBroadcaster* source) {
    if (source == &_audioProcessor.getProfileManager()) {
        refreshProfileMenu();
    }
}
