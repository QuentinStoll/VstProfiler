#pragma once

#include <JuceHeader.h>

#include "Components/CustomKnob.h"
#include "Components/CustomLevelMeter.h"
#include "Components/CustomToggleButton.h"
#include "Components/FileDropZone.h"
#include "Modules/BasicEqModule.h"

class ProfilerAudioProcessor;

class InputGatePanel : public juce::Component {
   public:
    explicit InputGatePanel(juce::AudioProcessorValueTreeState& apvts);
    ~InputGatePanel() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

   private:
    juce::Label _title{"", "Input / Gate"};
    juce::Label _summary{"", "Noise gate before the amp model."};
    CustomKnob _noiseGateKnob{"Noise Gate", 0.0f, 60.0f, 10.0f, "dB"};
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
    FileDropZone _ampDrop;
    CustomKnob _outputKnob{"Profil Output Volume", -12.0f, 12.0f, 0.0f, "dB"};
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
    juce::AudioProcessorValueTreeState& _apvts;
    juce::Label _title{"", "Cabinet / IR"};
    juce::Label _bypassLabel{"", "No IR loaded"};
    FileDropZone _irDrop;
    CustomKnob _lowCutKnob{"Low Cut", 20.0f, 250.0f, 80.0f, "Hz", 1.0f};

    void restoreCabNotes();
    void storeCabNotes();

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
    Gui::VerticalLevelMeter _outputMeter;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> _masterAttachment;

    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MasterVolumePanel)
};
