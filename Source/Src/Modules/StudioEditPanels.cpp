#include "Modules/StudioEditPanels.h"

#include "PluginProcessor.h"
#include "Stylesheet.h"

namespace {
constexpr const char* kCabMicProperty = "cabMic";
constexpr const char* kCabLowCutProperty = "cabLowCut";

void configurePanelTitle(juce::Label& label, const juce::String& text) {
    label.setText(text, juce::dontSendNotification);
    label.setFont(ProfilerStyle::Fonts::bold(13.0f));
    label.setColour(juce::Label::textColourId, ProfilerStyle::Colors::text);
    label.setJustificationType(juce::Justification::centredLeft);
    label.setInterceptsMouseClicks(false, false);
}

void configureBypassLabel(juce::Label& label) {
    label.setFont(ProfilerStyle::Fonts::bold(11.0f));
    label.setColour(juce::Label::textColourId, juce::Colour(0xffffb020));
    label.setJustificationType(juce::Justification::centredRight);
    label.setInterceptsMouseClicks(false, false);
}

FileDropZone::Options makeAmpDropOptions(ProfilerAudioProcessor& processor) {
    FileDropZone::Options options;
    options.title = "Amp Model";
    options.emptyDetail = "No Model Loaded";
    options.chooserTitle = "Select an Amp file";
    options.filePattern = "*.nam;*.json;*.txt";
    options.icon = CustomLookAndFeel::RigIcon::AmpHead;
    options.categoryColour = ProfilerStyle::Colors::rigAmp;
    options.loadFile = [&processor](const juce::File& file) {
        return processor.loadAmpFile(file);
    };
    options.unloadFile = [&processor]() {
        processor.unloadAmpFile();
    };
    options.isLoaded = [&processor]() {
        return processor.isAmpFileLoaded();
    };
    options.getCurrentFile = [&processor]() {
        return processor.getCurrentAmpFile();
    };
    options.successMessage = "Amp loaded";
    options.failureMessage = "Could not load amp file";
    return options;
}

FileDropZone::Options makeIrDropOptions(ProfilerAudioProcessor& processor) {
    FileDropZone::Options options;
    options.title = "Impulse Response";
    options.emptyDetail = "No IR Loaded";
    options.chooserTitle = "Select an IR file";
    options.filePattern = "*.wav;*.aiff;*.aif;*.flac";
    options.icon = CustomLookAndFeel::RigIcon::Cabinet;
    options.categoryColour = ProfilerStyle::Colors::rigCab;
    options.loadFile = [&processor](const juce::File& file) {
        return processor.loadIRFile(file);
    };
    options.unloadFile = [&processor]() {
        processor.unloadIRFile();
    };
    options.isLoaded = [&processor]() {
        return processor.isIRLoaded();
    };
    options.getCurrentFile = [&processor]() {
        return processor.getCurrentIRFile();
    };
    options.successMessage = "IR loaded";
    options.failureMessage = "Could not load IR file";
    return options;
}
}  // namespace

InputGatePanel::InputGatePanel(juce::AudioProcessorValueTreeState& apvts) {
    configurePanelTitle(_title, "Input / Gate");
    _summary.setFont(ProfilerStyle::Fonts::regular(11.0f));
    _summary.setColour(juce::Label::textColourId, ProfilerStyle::Colors::caption);
    _summary.setJustificationType(juce::Justification::centredLeft);
    _summary.setInterceptsMouseClicks(false, false);

    addAndMakeVisible(_title);
    addAndMakeVisible(_summary);
    addAndMakeVisible(_noiseGateKnob);
    _noiseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "noise", _noiseGateKnob.getSlider());
}

void InputGatePanel::paint(juce::Graphics&) {}

void InputGatePanel::resized() {
    auto area = getLocalBounds().reduced(10, 6);
    auto header = area.removeFromTop(20);
    _title.setBounds(header.removeFromLeft(120));
    _summary.setBounds(header);
    _noiseGateKnob.setBounds(area.withSizeKeepingCentre(108, juce::jmin(88, area.getHeight())));
}

AmpProfilerPanel::AmpProfilerPanel(ProfilerAudioProcessor& processor)
    : _ampDrop(makeAmpDropOptions(processor)) {
    configurePanelTitle(_title, "Amp Profiler");
    configureBypassLabel(_bypassLabel);
    addAndMakeVisible(_title);
    addAndMakeVisible(_bypassLabel);
    addAndMakeVisible(_ampDrop);
    addAndMakeVisible(_outputKnob);

    _outputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor._apvts, "gain", _outputKnob.getSlider());

    _ampDrop.onStatusMessage = [this](const juce::String& message, bool success) {
        if (onStatusMessage) {
            onStatusMessage(message, success);
        }
    };
    _ampDrop.onChanged = [this]() {
        refreshAssets();
        if (onAssetsChanged) {
            onAssetsChanged();
        }
    };

    refreshAssets();
}

void AmpProfilerPanel::refreshAssets() {
    _ampDrop.refresh();
    const auto loaded = _ampDrop.isFileLoaded();
    _outputKnob.setEnabled(loaded);
    _bypassLabel.setVisible(!loaded);
    resized();
}

void AmpProfilerPanel::paint(juce::Graphics&) {}

void AmpProfilerPanel::resized() {
    auto area = getLocalBounds().reduced(10, 6);
    auto header = area.removeFromTop(20);
    _bypassLabel.setBounds(header.removeFromRight(130));
    header.removeFromRight(8);
    _title.setBounds(header);
    area.removeFromTop(4);

    auto side = area.removeFromRight(112);
    area.removeFromRight(10);
    _ampDrop.setBounds(area);
    _outputKnob.setBounds(side.withSizeKeepingCentre(side.getWidth(), juce::jmin(92, side.getHeight())));
}

CabinetIrPanel::CabinetIrPanel(ProfilerAudioProcessor& processor)
    : _apvts(processor._apvts),
      _irDrop(makeIrDropOptions(processor)) {
    configurePanelTitle(_title, "Cabinet / IR");
    configureBypassLabel(_bypassLabel);
    addAndMakeVisible(_title);
    addAndMakeVisible(_bypassLabel);
    addAndMakeVisible(_irDrop);
    addAndMakeVisible(_micLabel);
    addAndMakeVisible(_micMenu);
    addAndMakeVisible(_lowCutKnob);

    _micLabel.setFont(ProfilerStyle::Fonts::regular(11.0f));
    _micLabel.setColour(juce::Label::textColourId, ProfilerStyle::Colors::caption);
    _micLabel.setJustificationType(juce::Justification::centredLeft);

    _micMenu.addItem("SM57", 1);
    _micMenu.addItem("MD421", 2);
    _micMenu.addItem("e609", 3);
    _micMenu.addItem("Condenser", 4);
    _micMenu.setSelectedId(1, juce::dontSendNotification);

    _micMenu.onChange = [this]() {
        storeCabNotes();
    };
    _lowCutKnob.getSlider().onValueChange = [this]() {
        storeCabNotes();
    };

    _irDrop.onStatusMessage = [this](const juce::String& message, bool success) {
        if (onStatusMessage) {
            onStatusMessage(message, success);
        }
    };
    _irDrop.onChanged = [this]() {
        refreshAssets();
        if (onAssetsChanged) {
            onAssetsChanged();
        }
    };

    restoreCabNotes();
    refreshAssets();
}

void CabinetIrPanel::refreshAssets() {
    _irDrop.refresh();
    const auto loaded = _irDrop.isFileLoaded();
    _micLabel.setEnabled(loaded);
    _micMenu.setEnabled(loaded);
    _lowCutKnob.setEnabled(loaded);
    _bypassLabel.setVisible(!loaded);
    resized();
}

void CabinetIrPanel::paint(juce::Graphics&) {}

void CabinetIrPanel::resized() {
    auto area = getLocalBounds().reduced(10, 6);
    auto header = area.removeFromTop(20);
    _bypassLabel.setBounds(header.removeFromRight(110));
    header.removeFromRight(8);
    _title.setBounds(header);
    area.removeFromTop(4);

    auto side = area.removeFromRight(112);
    area.removeFromRight(10);
    _irDrop.setBounds(area);

    _micLabel.setBounds(side.removeFromTop(12));
    side.removeFromTop(2);
    _micMenu.setBounds(side.removeFromTop(22));
    side.removeFromTop(4);
    _lowCutKnob.setBounds(side.withSizeKeepingCentre(side.getWidth(), juce::jmin(72, side.getHeight())));
}

void CabinetIrPanel::restoreCabNotes() {
    const auto micId = static_cast<int>(_apvts.state.getProperty(kCabMicProperty, 1));
    _micMenu.setSelectedId(juce::jlimit(1, 4, micId), juce::dontSendNotification);

    const auto lowCut = static_cast<float>(_apvts.state.getProperty(kCabLowCutProperty, 80.0));
    _lowCutKnob.getSlider().setValue(lowCut, juce::dontSendNotification);
}

void CabinetIrPanel::storeCabNotes() {
    _apvts.state.setProperty(kCabMicProperty, _micMenu.getSelectedId(), nullptr);
    _apvts.state.setProperty(kCabLowCutProperty, _lowCutKnob.getSlider().getValue(), nullptr);
}

EqPostFxPanel::EqPostFxPanel(juce::AudioProcessorValueTreeState& apvts)
    : _eqModule(apvts) {
    configurePanelTitle(_title, "EQ / Post-FX");
    configureBypassLabel(_bypassLabel);
    _eqEnabled.setLabelVisible(false);
    addAndMakeVisible(_title);
    addAndMakeVisible(_bypassLabel);
    addAndMakeVisible(_eqEnabled);
    addAndMakeVisible(_eqModule);
    _eqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, "isEqEnabled", _eqEnabled);

    _eqEnabled.onClick = [this]() {
        refreshBypassState();
    };
    refreshBypassState();
}

void EqPostFxPanel::refreshBypassState() {
    const auto enabled = _eqEnabled.getToggleState();
    _eqModule.setEnabled(enabled);
    _bypassLabel.setVisible(!enabled);
    resized();
    repaint();
}

void EqPostFxPanel::paint(juce::Graphics&) {}

void EqPostFxPanel::resized() {
    auto area = getLocalBounds().reduced(10, 6);
    auto header = area.removeFromTop(22);
    _eqEnabled.setBounds(header.removeFromRight(48).withSizeKeepingCentre(48, 20));
    header.removeFromRight(8);
    _bypassLabel.setBounds(header.removeFromRight(80));
    header.removeFromRight(8);
    _title.setBounds(header);

    const auto knobRowHeight = juce::jlimit(72, 92, area.getHeight());
    _eqModule.setBounds(area.withSizeKeepingCentre(area.getWidth(), knobRowHeight));
}
