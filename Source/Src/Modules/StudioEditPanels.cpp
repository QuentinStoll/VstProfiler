#include "Modules/StudioEditPanels.h"

#include <array>
#include <cmath>
#include <vector>

#include "PluginProcessor.h"
#include "Stylesheet.h"

namespace {
void configurePanelTitle(juce::Label& label, const juce::String& text) {
    label.setText(text, juce::dontSendNotification);
    label.setFont(ProfilerStyle::Fonts::bold(15.0f));
    label.setColour(juce::Label::textColourId, ProfilerStyle::Colors::text);
    label.setJustificationType(juce::Justification::centredLeft);
    label.setInterceptsMouseClicks(false, false);
}

void configureBypassLabel(juce::Label& label) {
    label.setFont(ProfilerStyle::Fonts::bold(12.0f));
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

namespace {
constexpr const char* kEqParameterIds[] = {"bass", "mid", "treble", "presence", "depth", "isEqEnabled"};
constexpr double kEqDisplaySampleRate = 48000.0;
constexpr int kEqCurvePoints = 256;
constexpr float kEqMinHz = 20.0f;
constexpr float kEqMaxHz = 20000.0f;
constexpr float kEqDisplayDb = 24.0f;

float eqBandGain(juce::AudioProcessorValueTreeState& apvts, const char* parameterId) {
    if (const auto* value = apvts.getRawParameterValue(parameterId)) {
        return value->load();
    }

    return 0.0f;
}

void addBandMagnitude(juce::dsp::IIR::Coefficients<float>::Ptr coefficients,
                      const double* frequencies,
                      double* magnitudes,
                      int numPoints) {
    if (coefficients == nullptr) {
        return;
    }

    std::vector<double> band(static_cast<size_t>(numPoints), 1.0);
    coefficients->getMagnitudeForFrequencyArray(frequencies, band.data(), static_cast<size_t>(numPoints), kEqDisplaySampleRate);
    for (int i = 0; i < numPoints; ++i) {
        magnitudes[i] *= band[static_cast<size_t>(i)];
    }
}
}  // namespace

EqResponseDisplay::EqResponseDisplay(juce::AudioProcessorValueTreeState& apvts)
    : _apvts(apvts) {
    for (const auto* parameterId : kEqParameterIds) {
        _apvts.addParameterListener(parameterId, this);
    }

    setInterceptsMouseClicks(false, false);
}

EqResponseDisplay::~EqResponseDisplay() {
    for (const auto* parameterId : kEqParameterIds) {
        _apvts.removeParameterListener(parameterId, this);
    }
}

void EqResponseDisplay::setBypassed(bool shouldBeBypassed) {
    if (_bypassed == shouldBeBypassed) {
        return;
    }

    _bypassed = shouldBeBypassed;
    repaint();
}

void EqResponseDisplay::parameterChanged(const juce::String&, float) {
    juce::MessageManager::callAsync([safeThis = juce::Component::SafePointer<EqResponseDisplay>(this)]() {
        if (safeThis != nullptr) {
            safeThis->repaint();
        }
    });
}

void EqResponseDisplay::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xff08080A));
    g.fillRoundedRectangle(bounds, 5.0f);
    g.setColour(juce::Colour(0xff2A2A32));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 5.0f, 1.0f);

    constexpr float leftGutter = 28.0f;
    constexpr float bottomGutter = 16.0f;
    constexpr float rightPad = 10.0f;
    constexpr float topPad = 8.0f;
    auto plot = juce::Rectangle<float>(bounds.getX() + leftGutter,
                                       bounds.getY() + topPad,
                                       bounds.getWidth() - leftGutter - rightPad,
                                       bounds.getHeight() - topPad - bottomGutter);
    if (plot.getWidth() < 12.0f || plot.getHeight() < 12.0f) {
        return;
    }

    const auto logSpan = std::log10(kEqMaxHz / kEqMinHz);
    const auto xForHz = [&plot, logSpan](float hz) {
        return plot.getX() + plot.getWidth() * std::log10(hz / kEqMinHz) / logSpan;
    };
    const auto yForDb = [&plot](float db) {
        const auto normalised = juce::jmap(juce::jlimit(-kEqDisplayDb, kEqDisplayDb, db),
                                           -kEqDisplayDb, kEqDisplayDb, 1.0f, 0.0f);
        return plot.getY() + normalised * plot.getHeight();
    };

    g.setColour(juce::Colour(0xff16161C));
    const float dbLines[] = {18.0f, 12.0f, 6.0f, -6.0f, -12.0f, -18.0f};
    for (const auto db : dbLines) {
        g.drawHorizontalLine(juce::roundToInt(yForDb(db)), plot.getX(), plot.getRight());
    }

    const float markerHz[] = {50.0f, 100.0f, 200.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f, 10000.0f};
    for (const auto hz : markerHz) {
        g.drawVerticalLine(juce::roundToInt(xForHz(hz)), plot.getY(), plot.getBottom());
    }

    g.setColour(juce::Colour(0xff3A3A46));
    g.drawHorizontalLine(juce::roundToInt(yForDb(0.0f)), plot.getX(), plot.getRight());

    std::array<double, kEqCurvePoints> frequencies{};
    std::array<double, kEqCurvePoints> magnitudes{};
    magnitudes.fill(1.0);
    const auto logMin = std::log10(static_cast<double>(kEqMinHz));
    const auto logMax = std::log10(static_cast<double>(kEqMaxHz));
    for (int i = 0; i < kEqCurvePoints; ++i) {
        const auto t = static_cast<double>(i) / static_cast<double>(kEqCurvePoints - 1);
        frequencies[static_cast<size_t>(i)] = std::pow(10.0, logMin + t * (logMax - logMin));
    }

    if (!_bypassed) {
        addBandMagnitude(juce::dsp::IIR::Coefficients<float>::makeLowShelf(
                             kEqDisplaySampleRate, 60.0f, 0.707f,
                             juce::Decibels::decibelsToGain(eqBandGain(_apvts, "depth"))),
                         frequencies.data(), magnitudes.data(), kEqCurvePoints);
        addBandMagnitude(juce::dsp::IIR::Coefficients<float>::makePeakFilter(
                             kEqDisplaySampleRate, 200.0f, 1.0f,
                             juce::Decibels::decibelsToGain(eqBandGain(_apvts, "bass"))),
                         frequencies.data(), magnitudes.data(), kEqCurvePoints);
        addBandMagnitude(juce::dsp::IIR::Coefficients<float>::makePeakFilter(
                             kEqDisplaySampleRate, 800.0f, 1.0f,
                             juce::Decibels::decibelsToGain(eqBandGain(_apvts, "mid"))),
                         frequencies.data(), magnitudes.data(), kEqCurvePoints);
        addBandMagnitude(juce::dsp::IIR::Coefficients<float>::makePeakFilter(
                             kEqDisplaySampleRate, 3200.0f, 1.0f,
                             juce::Decibels::decibelsToGain(eqBandGain(_apvts, "treble"))),
                         frequencies.data(), magnitudes.data(), kEqCurvePoints);
        addBandMagnitude(juce::dsp::IIR::Coefficients<float>::makeHighShelf(
                             kEqDisplaySampleRate, 8000.0f, 0.707f,
                             juce::Decibels::decibelsToGain(eqBandGain(_apvts, "presence"))),
                         frequencies.data(), magnitudes.data(), kEqCurvePoints);
    }

    juce::Path curve;
    juce::Path fill;
    fill.startNewSubPath(plot.getX(), yForDb(0.0f));
    for (int i = 0; i < kEqCurvePoints; ++i) {
        const auto t = static_cast<float>(i) / static_cast<float>(kEqCurvePoints - 1);
        const auto x = plot.getX() + t * plot.getWidth();
        const auto db = juce::Decibels::gainToDecibels(static_cast<float>(magnitudes[static_cast<size_t>(i)]), -kEqDisplayDb);
        const auto y = yForDb(db);
        if (i == 0) {
            curve.startNewSubPath(x, y);
        } else {
            curve.lineTo(x, y);
        }
        fill.lineTo(x, y);
    }
    fill.lineTo(plot.getRight(), yForDb(0.0f));
    fill.closeSubPath();

    {
        juce::Graphics::ScopedSaveState clip(g);
        g.reduceClipRegion(plot.toNearestInt());
        auto gradient = juce::ColourGradient(ProfilerStyle::Colors::rigEq.withAlpha(_bypassed ? 0.04f : 0.20f),
                                             plot.getCentreX(), plot.getY(),
                                             ProfilerStyle::Colors::rigEq.withAlpha(0.0f),
                                             plot.getCentreX(), plot.getBottom(),
                                             false);
        g.setGradientFill(gradient);
        g.fillPath(fill);
        g.setColour(ProfilerStyle::Colors::rigEq.withAlpha(_bypassed ? 0.32f : 0.96f));
        g.strokePath(curve, juce::PathStrokeType(_bypassed ? 1.1f : 1.7f,
                                                 juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));
    }

    g.setFont(ProfilerStyle::Fonts::regular(9.0f));
    g.setColour(ProfilerStyle::Colors::caption.withAlpha(0.55f));
    const float dbLabels[] = {12.0f, 0.0f, -12.0f};
    for (const auto db : dbLabels) {
        auto label = juce::Rectangle<float>(bounds.getX() + 2.0f, yForDb(db) - 6.0f, leftGutter - 4.0f, 12.0f);
        g.drawText(db == 0.0f ? "0" : juce::String(static_cast<int>(db)),
                   label, juce::Justification::centredRight, false);
    }

    const struct {
        float hz;
        const char* text;
    } freqLabels[] = {{100.0f, "100"}, {1000.0f, "1k"}, {10000.0f, "10k"}};
    for (const auto& freq : freqLabels) {
        g.drawText(freq.text,
                   juce::Rectangle<float>(xForHz(freq.hz) - 16.0f, plot.getBottom() + 1.0f, 32.0f, 14.0f),
                   juce::Justification::centred, false);
    }
}

InputGatePanel::InputGatePanel(juce::AudioProcessorValueTreeState& apvts) {
    configurePanelTitle(_title, "Input / Gate");
    configureBypassLabel(_bypassLabel);
    _summary.setFont(ProfilerStyle::Fonts::regular(13.0f));
    _summary.setColour(juce::Label::textColourId, ProfilerStyle::Colors::caption);
    _summary.setJustificationType(juce::Justification::centredLeft);
    _summary.setInterceptsMouseClicks(false, false);
    _gateEnabled.setLabelVisible(false);

    addAndMakeVisible(_title);
    addAndMakeVisible(_summary);
    addAndMakeVisible(_bypassLabel);
    addAndMakeVisible(_gateEnabled);
    addAndMakeVisible(_inputTrimKnob);
    addAndMakeVisible(_noiseGateKnob);
    _gateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, "isGateEnabled", _gateEnabled);
    _inputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "input", _inputTrimKnob.getSlider());
    _noiseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "noise", _noiseGateKnob.getSlider());

    _gateEnabled.onClick = [this]() {
        refreshBypassState();
    };
    refreshBypassState();
}

void InputGatePanel::refreshBypassState() {
    const auto enabled = _gateEnabled.getToggleState();
    _inputTrimKnob.setEnabled(true);
    _noiseGateKnob.setEnabled(enabled);
    _bypassLabel.setVisible(!enabled);
    resized();
}

void InputGatePanel::paint(juce::Graphics&) {}

void InputGatePanel::resized() {
    auto area = getLocalBounds().reduced(10, 6);
    auto header = area.removeFromTop(22);
    _gateEnabled.setBounds(header.removeFromRight(48).withSizeKeepingCentre(48, 20));
    header.removeFromRight(8);
    _bypassLabel.setBounds(header.removeFromRight(80));
    header.removeFromRight(8);
    _title.setBounds(header.removeFromLeft(140));
    _summary.setBounds(header);

    constexpr int knobWidth = 108;
    constexpr int gap = 24;
    auto row = juce::Rectangle<int>(knobWidth * 2 + gap, juce::jmin(88, area.getHeight())).withCentre(area.getCentre());
    _inputTrimKnob.setBounds(row.removeFromLeft(knobWidth));
    row.removeFromLeft(gap);
    _noiseGateKnob.setBounds(row);
}

AmpProfilerPanel::AmpProfilerPanel(ProfilerAudioProcessor& processor)
    : _ampDrop(makeAmpDropOptions(processor)) {
    configurePanelTitle(_title, "Amp Profiler");
    configureBypassLabel(_bypassLabel);
    _ampEnabled.setLabelVisible(false);
    addAndMakeVisible(_title);
    addAndMakeVisible(_bypassLabel);
    addAndMakeVisible(_ampEnabled);
    addAndMakeVisible(_ampDrop);
    addAndMakeVisible(_outputKnob);

    _ampEnabledAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor._apvts, "isAmpEnabled", _ampEnabled);
    _outputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor._apvts, "gain", _outputKnob.getSlider());

    _ampEnabled.onClick = [this]() {
        refreshAssets();
        if (onAssetsChanged) {
            onAssetsChanged();
        }
    };
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
    const auto enabled = _ampEnabled.getToggleState();
    _ampEnabled.setVisible(loaded);
    _outputKnob.setEnabled(loaded && enabled);
    _bypassLabel.setText(loaded ? "Bypassed" : "No model loaded", juce::dontSendNotification);
    _bypassLabel.setVisible(!loaded || !enabled);
    resized();
}

void AmpProfilerPanel::paint(juce::Graphics&) {}

void AmpProfilerPanel::resized() {
    auto area = getLocalBounds().reduced(10, 6);
    auto header = area.removeFromTop(22);
    _ampEnabled.setBounds(header.removeFromRight(48).withSizeKeepingCentre(48, 20));
    header.removeFromRight(8);
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
    : _irDrop(makeIrDropOptions(processor)) {
    configurePanelTitle(_title, "Cabinet / IR");
    configureBypassLabel(_bypassLabel);
    _cabEnabled.setLabelVisible(false);
    addAndMakeVisible(_title);
    addAndMakeVisible(_bypassLabel);
    addAndMakeVisible(_cabEnabled);
    addAndMakeVisible(_irDrop);
    addAndMakeVisible(_lowCutKnob);

    _cabEnabledAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor._apvts, "isCabEnabled", _cabEnabled);
    _lowCutAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor._apvts, "cabLowCut", _lowCutKnob.getSlider());

    _cabEnabled.onClick = [this]() {
        refreshAssets();
        if (onAssetsChanged) {
            onAssetsChanged();
        }
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

    refreshAssets();
}

void CabinetIrPanel::refreshAssets() {
    _irDrop.refresh();
    const auto loaded = _irDrop.isFileLoaded();
    const auto enabled = _cabEnabled.getToggleState();
    _cabEnabled.setVisible(loaded);
    _lowCutKnob.setEnabled(loaded && enabled);
    _bypassLabel.setText(loaded ? "Bypassed" : "No IR loaded", juce::dontSendNotification);
    _bypassLabel.setVisible(!loaded || !enabled);
    resized();
}

void CabinetIrPanel::paint(juce::Graphics&) {}

void CabinetIrPanel::resized() {
    auto area = getLocalBounds().reduced(10, 6);
    auto header = area.removeFromTop(22);
    _cabEnabled.setBounds(header.removeFromRight(48).withSizeKeepingCentre(48, 20));
    header.removeFromRight(8);
    _bypassLabel.setBounds(header.removeFromRight(110));
    header.removeFromRight(8);
    _title.setBounds(header);
    area.removeFromTop(4);

    auto side = area.removeFromRight(112);
    area.removeFromRight(10);
    _irDrop.setBounds(area);
    _lowCutKnob.setBounds(side.withSizeKeepingCentre(side.getWidth(), juce::jmin(92, side.getHeight())));
}

EqPostFxPanel::EqPostFxPanel(juce::AudioProcessorValueTreeState& apvts)
    : _eqDisplay(apvts),
      _eqModule(apvts) {
    configurePanelTitle(_title, "EQ / Post-FX");
    configureBypassLabel(_bypassLabel);
    _eqEnabled.setLabelVisible(false);
    addAndMakeVisible(_title);
    addAndMakeVisible(_bypassLabel);
    addAndMakeVisible(_eqEnabled);
    addAndMakeVisible(_eqDisplay);
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
    _eqDisplay.setBypassed(!enabled);
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
    area.removeFromTop(6);

    const auto graphHeight = juce::jlimit(64, 86, area.getHeight() - 78);
    _eqDisplay.setBounds(area.removeFromTop(graphHeight));
    area.removeFromTop(8);
    _eqModule.setBounds(area);
}

MasterVolumePanel::MasterVolumePanel(ProfilerAudioProcessor& processor)
    : _processor(processor) {
    configurePanelTitle(_title, "Master Volume");
    _summary.setFont(ProfilerStyle::Fonts::regular(13.0f));
    _summary.setColour(juce::Label::textColourId, ProfilerStyle::Colors::caption);
    _summary.setJustificationType(juce::Justification::centredLeft);
    _summary.setInterceptsMouseClicks(false, false);

    addAndMakeVisible(_title);
    addAndMakeVisible(_summary);
    addAndMakeVisible(_masterKnob);
    addAndMakeVisible(_outputTrimKnob);
    addAndMakeVisible(_outputMeter);

    _masterAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor._apvts, "master", _masterKnob.getSlider());
    _outputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor._apvts, "output", _outputTrimKnob.getSlider());
    startTimerHz(30);
}

MasterVolumePanel::~MasterVolumePanel() {
    stopTimer();
}

void MasterVolumePanel::paint(juce::Graphics&) {}

void MasterVolumePanel::resized() {
    auto area = getLocalBounds().reduced(10, 6);
    auto header = area.removeFromTop(22);
    _title.setBounds(header.removeFromLeft(150));
    _summary.setBounds(header);

    auto meter = area.removeFromRight(14).reduced(4, 8);
    area.removeFromRight(16);
    _outputMeter.setBounds(meter);

    constexpr int knobWidth = 108;
    constexpr int gap = 24;
    auto row = juce::Rectangle<int>(knobWidth * 2 + gap, juce::jmin(88, area.getHeight())).withCentre(area.getCentre());
    _masterKnob.setBounds(row.removeFromLeft(knobWidth));
    row.removeFromLeft(gap);
    _outputTrimKnob.setBounds(row);
}

void MasterVolumePanel::timerCallback() {
    _outputMeter.setLevel(_processor.getRmsLevelOutput());
}
