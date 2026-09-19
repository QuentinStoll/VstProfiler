#include "PluginEditor.h"

#include "ProfileManager.h"
#include "ProfilerConstantValues.h"
#include "Stylesheet.h"

ProfilerAudioProcessorEditor::ProfilerAudioProcessorEditor(ProfilerAudioProcessor& p)
    : AudioProcessorEditor(&p),
      _audioProcessor(p),
      _topBar(p),
      _inputGatePanel(p._apvts),
      _ampPanel(p),
      _cabinetPanel(p),
      _eqPanel(p._apvts),
      _libraryView(p) {
    SettingsView::applySavedBackgroundColour();
    setLookAndFeel(&_customLookAndFeel);

    setResizable(true, true);
    getConstrainer()->setFixedAspectRatio(editorWidth / static_cast<double>(editorHeight));
    setResizeLimits(800, 400, 1100, 550);
    setSize(editorWidth, editorHeight);

    addAndMakeVisible(_topBar);
    addAndMakeVisible(_signalChain);
    addAndMakeVisible(_editHost);
    _editHost.setInterceptsMouseClicks(false, true);
    _editHost.addAndMakeVisible(_inputGatePanel);
    _editHost.addAndMakeVisible(_ampPanel);
    _editHost.addAndMakeVisible(_cabinetPanel);
    _editHost.addAndMakeVisible(_eqPanel);

    _settingsHost.addAndMakeVisible(_settingsView);
    _settingsHost.addAndMakeVisible(_resetButton);
    _settingsHost.addAndMakeVisible(_exportButton);
    addChildComponent(_settingsHost);
    addChildComponent(_libraryView);

    _exportViewport.getVerticalScrollBar().setColour(juce::ScrollBar::thumbColourId, ProfilerStyle::Colors::accent);
    _exportViewport.setViewedComponent(&_exportModule, false);
    _exportViewport.setScrollBarsShown(true, false);
    addChildComponent(_exportViewport);
    addChildComponent(_closeOverlayButton);
    addChildComponent(_notificationBanner);

    _topBar.onSettingsClicked = [this]() {
        showOverlay(_overlayMode == OverlayMode::Settings ? OverlayMode::None : OverlayMode::Settings);
    };
    _topBar.onLibraryClicked = [this]() {
        showOverlay(_overlayMode == OverlayMode::Library ? OverlayMode::None : OverlayMode::Library);
    };
    _closeOverlayButton.onClick = [this]() {
        showStudio();
    };
    _resetButton.onClick = [this]() {
        showStatus(_topBar.restoreDefaults(), true);
        updateChainStatus();
        _ampPanel.refreshAssets();
        _cabinetPanel.refreshAssets();
        _eqPanel.refreshBypassState();
    };
    _exportButton.onClick = [this]() {
        showOverlay(OverlayMode::Export);
    };

    _ampPanel.onStatusMessage = [this](const juce::String& message, bool success) {
        showStatus(message, success);
    };
    _ampPanel.onAssetsChanged = [this]() {
        updateChainStatus();
    };
    _cabinetPanel.onStatusMessage = [this](const juce::String& message, bool success) {
        showStatus(message, success);
    };
    _cabinetPanel.onAssetsChanged = [this]() {
        updateChainStatus();
    };

    _signalChain.onBlockSelected = [this](SignalChainStrip::BlockId blockId) {
        showEditPanel(blockId);
    };

    _exportModule.onExportClicked = [this](const juce::NamedValueSet& values, const juce::File& destinationFile) {
        exportProfil(values, destinationFile);
    };
    _exportModule.onCreateProfileClicked = [this](const juce::NamedValueSet& values) {
        createProfil(values);
    };
    _exportModule.onCancelClicked = [this]() {
        showStudio();
    };

    _audioProcessor.getProfileManager().addChangeListener(this);
    _audioProcessor._apvts.addParameterListener("isEqEnabled", this);
    _audioProcessor._apvts.addParameterListener("noise", this);

    showEditPanel(SignalChainStrip::BlockId::AmpProfiler);
    updateChainStatus();
}

ProfilerAudioProcessorEditor::~ProfilerAudioProcessorEditor() {
    _audioProcessor.getProfileManager().removeChangeListener(this);
    _audioProcessor._apvts.removeParameterListener("isEqEnabled", this);
    _audioProcessor._apvts.removeParameterListener("noise", this);
    setLookAndFeel(nullptr);
}

void ProfilerAudioProcessorEditor::paint(juce::Graphics& g) {
    ProfilerStyle::Surfaces::fillWindow(g, getLocalBounds().toFloat());

    if (_overlayMode != OverlayMode::None) {
        return;
    }

    if (auto* laf = dynamic_cast<CustomLookAndFeel*>(&getLookAndFeel())) {
        laf->drawRaisedPanel(g, _editHost.getBounds().toFloat());
    } else {
        ProfilerStyle::Surfaces::fillPanel(g, _editHost.getBounds().toFloat());
    }
}

void ProfilerAudioProcessorEditor::resized() {
    auto area = getLocalBounds().reduced(12);
    _topBar.setBounds(area.removeFromTop(48));
    area.removeFromTop(8);

    if (_overlayMode == OverlayMode::None) {
        constexpr int editHeight = 126;
        _editHost.setBounds(area.removeFromBottom(juce::jmin(editHeight, juce::jmax(0, area.getHeight() - 160))));
        area.removeFromBottom(8);
        _signalChain.setBounds(area);

        auto panelBounds = _editHost.getLocalBounds();
        _inputGatePanel.setBounds(panelBounds);
        _ampPanel.setBounds(panelBounds);
        _cabinetPanel.setBounds(panelBounds);
        _eqPanel.setBounds(panelBounds);
        _closeOverlayButton.setBounds({});
    } else {
        auto header = area.removeFromTop(36);
        _closeOverlayButton.setBounds(header.removeFromRight(88));
        area.removeFromTop(8);

        _settingsHost.setBounds(area);
        _libraryView.setBounds(area);
        _exportViewport.setBounds(area);

        auto settingsArea = _settingsHost.getLocalBounds().reduced(12);
        auto actions = settingsArea.removeFromBottom(40);
        _exportButton.setBounds(actions.removeFromRight(140));
        actions.removeFromRight(10);
        _resetButton.setBounds(actions.removeFromRight(140));
        settingsArea.removeFromBottom(12);
        _settingsView.setBounds(settingsArea);

        const auto exportHeight = juce::jmax(area.getHeight(), _exportModule.getRequiredHeight(area.getWidth()));
        _exportModule.setBounds(0, 0, area.getWidth(), exportHeight);
    }

    const auto bannerWidth = juce::jmin(_notificationBanner.getIdealWidth(), juce::jmax(220, getWidth() - 50));
    _notificationBanner.setBounds(getLocalBounds()
                                      .withSizeKeepingCentre(bannerWidth, _notificationBanner.getIdealHeight())
                                      .withRightX(getWidth() - 22)
                                      .withY(18));
}

void ProfilerAudioProcessorEditor::showStudio() {
    _overlayMode = OverlayMode::None;
    _signalChain.setVisible(true);
    _editHost.setVisible(true);
    _settingsHost.setVisible(false);
    _libraryView.setVisible(false);
    _exportViewport.setVisible(false);
    _closeOverlayButton.setVisible(false);
    resized();
    repaint();
}

void ProfilerAudioProcessorEditor::showOverlay(OverlayMode mode) {
    if (mode == OverlayMode::None) {
        showStudio();
        return;
    }

    if (mode == OverlayMode::Export) {
        const auto exportName = getDefaultExportName();
        auto values = _audioProcessor.getProfileManager().getCurrentProfileValues(exportName);
        values.set("irPath", _audioProcessor.isIRLoaded()
                                 ? _audioProcessor.getCurrentIRFile().getFullPathName()
                                 : juce::String{});
        values.set("ampPath", _audioProcessor.isAmpFileLoaded()
                                  ? _audioProcessor.getCurrentAmpFile().getFullPathName()
                                  : juce::String{});
        _exportModule.setExportValues(values, getDefaultExportFile(exportName));
        _exportViewport.setViewPosition(0, 0);
    }

    _overlayMode = mode;
    _signalChain.setVisible(false);
    _editHost.setVisible(false);
    _settingsHost.setVisible(mode == OverlayMode::Settings);
    _libraryView.setVisible(mode == OverlayMode::Library);
    _exportViewport.setVisible(mode == OverlayMode::Export);
    _closeOverlayButton.setVisible(true);
    resized();
    repaint();
}

void ProfilerAudioProcessorEditor::showEditPanel(SignalChainStrip::BlockId blockId) {
    _inputGatePanel.setVisible(blockId == SignalChainStrip::BlockId::InputGate);
    _ampPanel.setVisible(blockId == SignalChainStrip::BlockId::AmpProfiler);
    _cabinetPanel.setVisible(blockId == SignalChainStrip::BlockId::CabinetIr);
    _eqPanel.setVisible(blockId == SignalChainStrip::BlockId::EqPostFx);
}

void ProfilerAudioProcessorEditor::updateChainStatus() {
    const auto ampLoaded = _audioProcessor.isAmpFileLoaded();
    const auto irLoaded = _audioProcessor.isIRLoaded();
    const auto eqEnabled = _audioProcessor._apvts.getRawParameterValue("isEqEnabled") != nullptr
                           && _audioProcessor._apvts.getRawParameterValue("isEqEnabled")->load() > 0.5f;
    const auto noise = _audioProcessor._apvts.getRawParameterValue("noise");

    _signalChain.setBlockLed(SignalChainStrip::BlockId::InputGate, true);
    _signalChain.setBlockLed(SignalChainStrip::BlockId::AmpProfiler, ampLoaded);
    _signalChain.setBlockLed(SignalChainStrip::BlockId::CabinetIr, irLoaded);
    _signalChain.setBlockLed(SignalChainStrip::BlockId::EqPostFx, eqEnabled);

    _signalChain.setBlockSubtitle(SignalChainStrip::BlockId::InputGate,
                                  noise != nullptr ? "Gate " + juce::String(noise->load(), 1) + " dB" : "Noise Gate");
    _signalChain.setBlockSubtitle(SignalChainStrip::BlockId::AmpProfiler,
                                  ampLoaded ? _audioProcessor.getCurrentAmpFile().getFileName() : "Load model");
    _signalChain.setBlockSubtitle(SignalChainStrip::BlockId::CabinetIr,
                                  irLoaded ? _audioProcessor.getCurrentIRFile().getFileName() : "Load IR");
    _signalChain.setBlockSubtitle(SignalChainStrip::BlockId::EqPostFx, eqEnabled ? "Enabled" : "Bypassed");
}

void ProfilerAudioProcessorEditor::showStatus(const juce::String& message, bool success) {
    _notificationBanner.clearAction();
    _notificationBanner.showMessage(message,
                                    success ? NotificationBanner::Type::Success : NotificationBanner::Type::Error,
                                    5000);
    resized();
}

void ProfilerAudioProcessorEditor::exportProfil(const juce::NamedValueSet& values, const juce::File& destinationFile) {
    juce::String errorMessage;
    if (_audioProcessor.getProfileManager().exportProfile(values, destinationFile, &errorMessage)) {
        showStudio();
        showStatus("Profile exported successfully", true);
    } else {
        showStatus(errorMessage, false);
    }
}

void ProfilerAudioProcessorEditor::createProfil(const juce::NamedValueSet& values) {
    juce::String errorMessage;
    if (_audioProcessor.getProfileManager().createProfile(values, &errorMessage)) {
        showStudio();
        showStatus("Profile created successfully", true);
    } else {
        showStatus(errorMessage, false);
    }
}

juce::String ProfilerAudioProcessorEditor::getDefaultExportName() const {
    const auto& profileManager = _audioProcessor.getProfileManager();
    if (const auto* profile = profileManager.getProfile(profileManager.getCurrentProfileIndex())) {
        if (profile->name.trim().isNotEmpty()) {
            return profile->name;
        }
    }

    return "Exported Profile";
}

juce::File ProfilerAudioProcessorEditor::getDefaultExportFile(const juce::String& profileName) const {
    auto fileName = juce::File::createLegalFileName(profileName.trim());
    if (fileName.isEmpty()) {
        fileName = "Exported Profile";
    }

    return juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
        .getChildFile(fileName + ProfileManager::profileFileExtension);
}

void ProfilerAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster* source) {
    if (source == &_audioProcessor.getProfileManager()) {
        _ampPanel.refreshAssets();
        _cabinetPanel.refreshAssets();
        _eqPanel.refreshBypassState();
        updateChainStatus();
    }
}

void ProfilerAudioProcessorEditor::parameterChanged(const juce::String& parameterID, float /*newValue*/) {
    if (parameterID == "isEqEnabled" || parameterID == "noise") {
        juce::MessageManager::callAsync([safeThis = juce::Component::SafePointer<ProfilerAudioProcessorEditor>(this)]() {
            if (safeThis != nullptr) {
                safeThis->_eqPanel.refreshBypassState();
                safeThis->updateChainStatus();
            }
        });
    }
}
