#pragma once

#include <JuceHeader.h>

#include "Components/CustomKnob.h"
#include "Components/CustomLevelMeter.h"
#include "Components/CustomToggleButton.h"
#include "Components/FileDropZone.h"
#include "Modules/BasicEqModule.h"

class ProfilerAudioProcessor;

class EqResponseDisplay : public juce::Component,
                          private juce::AudioProcessorValueTreeState::Listener {
   public:
    explicit EqResponseDisplay(juce::AudioProcessorValueTreeState& apvts);
    ~EqResponseDisplay() override;

    void paint(juce::Graphics& g) override;
    void setBypassed(bool shouldBeBypassed);

   private:
    juce::AudioProcessorValueTreeState& _apvts;
    bool _bypassed = false;

    void parameterChanged(const juce::String& parameterID, float newValue) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EqResponseDisplay)
};

class InputGatePanel : public juce::Component {
   public:
    explicit InputGatePanel(juce::AudioProcessorValueTreeState& apvts);
    ~InputGatePanel() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void refreshBypassState();

   private:
    juce::Label _title{"", "Input / Gate"};
    juce::Label _summary{"", "Input trim and noise gate before the amp model."};
    juce::Label _bypassLabel{"", "Bypassed"};
    CustomToggleButton _gateEnabled{"Gate Enabled"};
    CustomKnob _inputTrimKnob{"Input Trim", -12.0f, 12.0f, 0.0f, "dB"};
    CustomKnob _noiseGateKnob{"Noise Gate", 0.0f, 60.0f, 10.0f, "dB"};
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> _gateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> _inputAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> _noiseAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InputGatePanel)
};

class AmpProfilerPanel : public juce::Component {
   public:
    explicit AmpProfilerPanel(ProfilerAudioProcessor& processor);
    ~AmpProfilerPanel() override = default;

    void refreshAssets();

    std::function<void(const juce::String&, bool)> onStatusMessage;
    std::function<void()> onAssetsChanged;

    void paint(juce::Graphics& g) override;
    void resized() override;

   private:
    juce::Label _title{"", "Amp Profiler"};
    juce::Label _bypassLabel{"", "No model loaded"};
    CustomToggleButton _ampEnabled{"Amp Enabled"};
    FileDropZone _ampDrop;
    CustomKnob _outputKnob{"Profil Output Volume", -12.0f, 12.0f, 0.0f, "dB"};
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> _ampEnabledAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> _outputAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AmpProfilerPanel)
};

class CabinetIrPanel : public juce::Component {
   public:
    explicit CabinetIrPanel(ProfilerAudioProcessor& processor);
    ~CabinetIrPanel() override = default;

    void refreshAssets();

    std::function<void(const juce::String&, bool)> onStatusMessage;
    std::function<void()> onAssetsChanged;

    void paint(juce::Graphics& g) override;
    void resized() override;

   private:
    juce::Label _title{"", "Cabinet / IR"};
    juce::Label _bypassLabel{"", "No IR loaded"};
    CustomToggleButton _cabEnabled{"Cab Enabled"};
    FileDropZone _irDrop;
    CustomKnob _lowCutKnob{"Low Cut", 20.0f, 250.0f, 80.0f, "Hz", 1.0f};
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> _cabEnabledAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> _lowCutAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CabinetIrPanel)
};

class EqPostFxPanel : public juce::Component {
   public:
    explicit EqPostFxPanel(juce::AudioProcessorValueTreeState& apvts);
    ~EqPostFxPanel() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void refreshBypassState();

   private:
    juce::Label _title{"", "EQ / Post-FX"};
    juce::Label _bypassLabel{"", "Bypassed"};
    CustomToggleButton _eqEnabled{"EQ Enabled"};
    EqResponseDisplay _eqDisplay;
    BasicEqModule _eqModule;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> _eqAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EqPostFxPanel)
};

class MasterVolumePanel : public juce::Component,
                          private juce::Timer {
   public:
    explicit MasterVolumePanel(ProfilerAudioProcessor& processor);
    ~MasterVolumePanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

   private:
    ProfilerAudioProcessor& _processor;
    juce::Label _title{"", "Master Volume"};
    juce::Label _summary{"", "Output level after the signal chain."};
    CustomKnob _masterKnob{"Master", 0.0f, 100.0f, 50.0f, "%", 1.0f};
    CustomKnob _outputTrimKnob{"Output Trim", -12.0f, 12.0f, 0.0f, "dB"};
    Gui::VerticalLevelMeter _outputMeter;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> _masterAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> _outputAttachment;

    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MasterVolumePanel)
};
