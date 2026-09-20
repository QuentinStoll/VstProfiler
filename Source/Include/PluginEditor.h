#pragma once

#include <JuceHeader.h>

#include "Components/CustomLookAndFeel.h"
#include "Components/NotificationBanner.h"
#include "Components/SignalChainStrip.h"
#include "Modules/ExportProfilModule.h"
#include "Modules/StudioEditPanels.h"
#include "Modules/StudioTopBar.h"
#include "PluginProcessor.h"
#include "Views/ProfilView.h"
#include "Views/SettingsView.h"

class ProfilerAudioProcessorEditor : public juce::AudioProcessorEditor,
                                     private juce::ChangeListener,
                                     private juce::AudioProcessorValueTreeState::Listener,
                                     private juce::Timer {
   public:
    ProfilerAudioProcessorEditor(ProfilerAudioProcessor&);
    ~ProfilerAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void parentHierarchyChanged() override;
    bool keyPressed(const juce::KeyPress& key) override;

   private:
    enum class OverlayMode {
        None,
        Settings,
        Library,
        Export
    };

    ProfilerAudioProcessor& _audioProcessor;
    CustomLookAndFeel _customLookAndFeel;
    juce::Component _content;

    StudioTopBar _topBar;
    SignalChainStrip _signalChain;
    juce::Component _editHost;
    InputGatePanel _inputGatePanel;
    AmpProfilerPanel _ampPanel;
    CabinetIrPanel _cabinetPanel;
    EqPostFxPanel _eqPanel;
    MasterVolumePanel _masterPanel;

    SettingsView _settingsView;
    juce::TextButton _resetButton{"Reset Chain"};
    juce::TextButton _exportButton{"Export Profile"};
    juce::TextButton _closeOverlayButton{"Close"};
    juce::Component _settingsHost;

    ProfilView _libraryView;
    juce::Viewport _exportViewport;
    ExportProfilModule _exportModule;
    NotificationBanner _notificationBanner;
    juce::TooltipWindow _tooltipWindow{this, 700};
    OverlayMode _overlayMode = OverlayMode::None;

    void applyStandaloneWindowChrome();

    void showStudio();
    void showOverlay(OverlayMode mode);
    void showEditPanel(SignalChainStrip::BlockId blockId);
    void updateChainStatus();
    void toggleBlockBypass(SignalChainStrip::BlockId blockId);
    void showStatus(const juce::String& message, bool success);
    void exportProfil(const juce::NamedValueSet& values, const juce::File& destinationFile);
    void createProfil(const juce::NamedValueSet& values);
    juce::String getDefaultExportName() const;
    juce::File getDefaultExportFile(const juce::String& profileName) const;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProfilerAudioProcessorEditor)
};
