#include "PluginProcessor.h"

#include <algorithm>
#include <cmath>
#include <common/TracyColor.hpp>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

#define RTNEURAL_DEFAULT_STATIC 1
#define RTNEURAL_ENABLE_LSTM 1
#define RTNEURAL_ENABLE_GRU 1
#define RTNEURAL_ENABLE_DENSE 1

#include <RTNeural/RTNeural.h>

#include <tracy/Tracy.hpp>

#include "Logging.h"
#if !(defined(PROFILER_HEADLESS_TESTS) && PROFILER_HEADLESS_TESTS)
#include "PluginEditor.h"
#endif
#include "UiSettings.h"

float ProfilerAudioProcessor::getParameterValue(const std::atomic<float>* parameter, float fallback) noexcept {
    return parameter != nullptr ? parameter->load() : fallback;
}

bool ProfilerAudioProcessor::isCompatibleAmpModel(const RTNeural::Model<float>& model) {
    return !model.layers.empty() &&
           model.getInSize() == 1 &&
           model.getOutSize() >= 1;
}

float ProfilerAudioProcessor::getMasterGainLinear(float masterPercent) noexcept {
    const auto percent = juce::jlimit(0.0f, 100.0f, masterPercent);

    if (percent <= 0.0f) {
        return 0.0f;
    }

    if (percent <= 50.0f) {
        return percent / 50.0f;
    }

    return juce::Decibels::decibelsToGain(juce::jmap(percent, 50.0f, 100.0f, 0.0f, 12.0f));
}

//==============================================================================
ProfilerAudioProcessor::ProfilerAudioProcessor(juce::File profileDirectory,
                                               juce::File playViewSettingsFile)
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(
          BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
              .withInput("Input", juce::AudioChannelSet::mono(), true)
#endif
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
              ),
      _profileManager(_apvts, std::move(profileDirectory), std::move(playViewSettingsFile))
#else
    : _profileManager(_apvts, std::move(profileDirectory), std::move(playViewSettingsFile))
#endif
{
    Log::logSystemInfoOnFileStart = (bool)(UiSettings::loadHardwareInfoSetting() - 1);
    Log::LogConfig config = Log::LogConfig::fromFile((juce::File)("/home/krt/dev/EIP/VstProfiler/.config/log_settings.json"));
    if (Log::LogRegistry::find(config.name) == nullptr) {
        Log::LogRegistry::create(config.name, config);
    }

    _masterParam = _apvts.getRawParameterValue("master");
    _gainParam = _apvts.getRawParameterValue("gain");
    _noiseParam = _apvts.getRawParameterValue("noise");
    _inputParam = _apvts.getRawParameterValue("input");
    _outputParam = _apvts.getRawParameterValue("output");

    for (int band = 0; band < EqBands::count; ++band) {
        _eqGainParams[static_cast<size_t>(band)] = _apvts.getRawParameterValue(EqBands::specs[band].gainId);
        _eqFreqParams[static_cast<size_t>(band)] = _apvts.getRawParameterValue(EqBands::specs[band].freqId);
    }

    _isMuteParam = _apvts.getRawParameterValue("isMute");
    _isEqEnabledParam = _apvts.getRawParameterValue("isEqEnabled");
    _isGateEnabledParam = _apvts.getRawParameterValue("isGateEnabled");
    _isAmpEnabledParam = _apvts.getRawParameterValue("isAmpEnabled");
    _isCabEnabledParam = _apvts.getRawParameterValue("isCabEnabled");
    _cabLowCutParam = _apvts.getRawParameterValue("cabLowCut");
    _isPedalEnabledParam = _apvts.getRawParameterValue("isPedalEnabled");
    _pedalDriveParam = _apvts.getRawParameterValue("pedalDrive");
    _pedalToneParam = _apvts.getRawParameterValue("pedalTone");
    _pedalLevelParam = _apvts.getRawParameterValue("pedalLevel");
    _apvts.state.setProperty("chainLayout", static_cast<int>(_chainLayoutPacked.load()), nullptr);
    _spectra.open();
}

ProfilerAudioProcessor::~ProfilerAudioProcessor() = default;

//==============================================================================
const juce::String ProfilerAudioProcessor::getName() const {
    return JucePlugin_Name;
}

bool ProfilerAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool ProfilerAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool ProfilerAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double ProfilerAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int ProfilerAudioProcessor::getNumPrograms() { return 1; }
int ProfilerAudioProcessor::getCurrentProgram() { return 0; }
void ProfilerAudioProcessor::setCurrentProgram(int /*index*/) {}
const juce::String ProfilerAudioProcessor::getProgramName(int /*index*/) { return {}; }
void ProfilerAudioProcessor::changeProgramName(int /*index*/, const juce::String& /*newName*/) {}

//==============================================================================
void ProfilerAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32)samplesPerBlock;
    spec.numChannels = 1;

    _convolver.prepare(spec);

    _inputTrim.prepare(spec);
    _inputTrim.setRampDurationSeconds(PARAMETER_RAMP_SECONDS);
    _inputTrim.setGainDecibels(getParameterValue(_inputParam, 0.0f));

    _chain.reset();
    _chain.prepare(spec);

    _chain.get<Gain>().setGainDecibels(getParameterValue(_gainParam, 0.0f));
    _chain.get<Gain>().setRampDurationSeconds(PARAMETER_RAMP_SECONDS);

    _masterVolume.prepare(spec);
    _masterVolume.setRampDurationSeconds(PARAMETER_RAMP_SECONDS);
    _masterVolume.setGainLinear(getMasterGainLinear(getParameterValue(_masterParam, 50.0f)));

    _outputTrim.prepare(spec);
    _outputTrim.setRampDurationSeconds(PARAMETER_RAMP_SECONDS);
    _outputTrim.setGainDecibels(getParameterValue(_outputParam, 0.0f));

    _chain.get<NoiseGate>().setThreshold(getParameterValue(_noiseParam, 10.0f) - 60.0f);
    _chain.get<NoiseGate>().setAttack(5.0f);
    _chain.get<NoiseGate>().setRelease(100.0f);
    _chain.get<NoiseGate>().setRatio(10.0f);
    _chain.setBypassed<NoiseGate>(getParameterValue(_isGateEnabledParam, 1.0f) <= 0.5f || getParameterValue(_noiseParam, 0.0f) <= 0.0f);

    const bool eqEnabled = getParameterValue(_isEqEnabledParam, 1.0f) > 0.5f;

    _eqChain.reset();
    _eqChain.prepare(spec);
    _eqChain.setBypassed<LowShelf>(!eqEnabled);
    _eqChain.setBypassed<Peak1>(!eqEnabled);
    _eqChain.setBypassed<Peak2>(!eqEnabled);
    _eqChain.setBypassed<Peak3>(!eqEnabled);
    _eqChain.setBypassed<Peak4>(!eqEnabled);
    _eqChain.setBypassed<HighShelf>(!eqEnabled);

    for (int band = 0; band < EqBands::count; ++band) {
        const auto index = static_cast<size_t>(band);
        _eqGainSmoothed[index].reset(sampleRate, PARAMETER_RAMP_SECONDS);
        _eqFreqSmoothed[index].reset(sampleRate, PARAMETER_RAMP_SECONDS);
        _eqGainSmoothed[index].setCurrentAndTargetValue(
            getParameterValue(_eqGainParams[index], EqBands::specs[band].defaultDb));
        _eqFreqSmoothed[index].setCurrentAndTargetValue(
            getParameterValue(_eqFreqParams[index], EqBands::specs[band].defaultHz));
    }
    updateEqCoefficients();
    _eqCoeffsDirty = false;
    _eqChain.reset();

    _cabLowCutSmoothed.reset(sampleRate, PARAMETER_RAMP_SECONDS);
    _cabLowCutSmoothed.setCurrentAndTargetValue(getParameterValue(_cabLowCutParam, 80.0f));
    _cabLowCut.reset();
    _cabLowCut.prepare(spec);
    updateCabLowCutCoefficients();
    _cabLowCut.reset();

    _pedalToneFilter.reset();
    _pedalToneFilter.prepare(spec);
    updatePedalToneCoefficients();
    _pedalToneFilter.reset();

    *_dcBlocker.state = juce::dsp::IIR::ArrayCoefficients<float>::makeHighPass(sampleRate, 35.0f);
    _dcBlocker.prepare(spec);
    _dcBlocker.reset();

    {
        const juce::ScopedLock ampLock(_ampModelLock);
        if (_neuralAmp != nullptr) {
            _neuralAmp->reset();
            _ampLoaded = true;
        }
    }
}

void ProfilerAudioProcessor::releaseResources() {
    _chain.reset();
    _eqChain.reset();
    _cabLowCut.reset();
    _pedalToneFilter.reset();
    _inputTrim.reset();
    _masterVolume.reset();
    _outputTrim.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ProfilerAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const {
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()) {
        return false;
    }

    // The processor accepts one mono input and exposes a stereo output so the
    // processed mono amp signal can be heard identically on both sides.
#if !JucePlugin_IsSynth
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::mono() ||
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
#endif

    return true;
#endif
}
#endif

/**
    Performs the real-time audio processing.
    This implementation handles gain scaling, a main Dsp chain,
    an oversampled non-linear amp stage, and an IR convolution stage.
*/
void ProfilerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer& /*midiMessages*/) {
    ZoneScopedNC("test2", tracy::Color::Purple);

    juce::ScopedNoDenormals noDenormals;

    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();
    if (numChannels <= 0 || numSamples <= 0) {
        return;
    }

    _rmsLevelInput.store(juce::Decibels::gainToDecibels(buffer.getRMSLevel(0, 0, numSamples), -60.0f),
                         std::memory_order_relaxed);

    // Handle Mute
    if (getParameterValue(_isMuteParam, 0.0f) > 0.5f) {
        buffer.clear();
        _rmsLevelOutput.store(-60.0f, std::memory_order_relaxed);
        return;
    }

    // Handle any incoming MIDI messages (e.g., for parameter automation)
    for (auto i = getTotalNumInputChannels(); i < juce::jmin(getTotalNumOutputChannels(), numChannels); ++i)
        buffer.clear(i, 0, numSamples);

    _inputTrim.setGainDecibels(getParameterValue(_inputParam, 0.0f));
    _chain.get<Gain>().setGainDecibels(getParameterValue(_gainParam, 0.0f));
    const auto noiseGateValue = getParameterValue(_noiseParam, 0.0f);
    const bool gateEnabled = getParameterValue(_isGateEnabledParam, 1.0f) > 0.5f;
    _chain.get<NoiseGate>().setThreshold(noiseGateValue - 60.0f);
    // A zero control value is a true bypass, rather than a -60 dB gate.
    _chain.setBypassed<NoiseGate>(!gateEnabled || noiseGateValue <= 0.0f);

    bool eqSmoothing = false;
    for (int band = 0; band < EqBands::count; ++band) {
        const auto index = static_cast<size_t>(band);
        _eqGainSmoothed[index].setTargetValue(getParameterValue(_eqGainParams[index], EqBands::specs[band].defaultDb));
        _eqFreqSmoothed[index].setTargetValue(
            getParameterValue(_eqFreqParams[index], EqBands::specs[band].defaultHz));
        eqSmoothing = eqSmoothing || _eqGainSmoothed[index].isSmoothing() || _eqFreqSmoothed[index].isSmoothing();
    }

    const bool eqEnabled = getParameterValue(_isEqEnabledParam, 1.0f) > 0.5f;
    _eqChain.setBypassed<LowShelf>(!eqEnabled);
    _eqChain.setBypassed<Peak1>(!eqEnabled);
    _eqChain.setBypassed<Peak2>(!eqEnabled);
    _eqChain.setBypassed<Peak3>(!eqEnabled);
    _eqChain.setBypassed<Peak4>(!eqEnabled);
    _eqChain.setBypassed<HighShelf>(!eqEnabled);

    if (eqEnabled && (_eqCoeffsDirty || eqSmoothing)) {
        updateEqCoefficients();
        _eqCoeffsDirty = eqSmoothing;
    }

    juce::dsp::AudioBlock<float> block(buffer);
    auto monoBlock = block.getSingleChannelBlock(0);
    juce::dsp::ProcessContextReplacing<float> context(monoBlock);
    _inputTrim.process(context);
    processGateStage(context);

    _cabLowCutSmoothed.setTargetValue(getParameterValue(_cabLowCutParam, 80.0f));
    const auto layout = SignalChain::Layout::fromPacked(_chainLayoutPacked.load(std::memory_order_relaxed));
    for (int slot = SignalChain::firstMovableSlot; slot <= SignalChain::lastMovableSlot; ++slot) {
        switch (layout.atSlot(slot)) {
            case SignalChain::Stage::Cab:
                processCabStage(context);
                break;
            case SignalChain::Stage::Eq:
                processEqStage(context);
                break;
            case SignalChain::Stage::Pedal:
                processPedalStage(buffer, context, numSamples);
                break;
            case SignalChain::Stage::Amp:
                processAmpStage(buffer, context, numSamples);
                break;
            case SignalChain::Stage::Empty:
            default:
                break;
        }
    }

    _cabLowCutSmoothed.skip(numSamples);
    for (auto& smoothed : _eqGainSmoothed) {
        smoothed.skip(numSamples);
    }
    for (auto& smoothed : _eqFreqSmoothed) {
        smoothed.skip(numSamples);
    }

    _masterVolume.setGainLinear(getMasterGainLinear(getParameterValue(_masterParam, 50.0f)));
    _masterVolume.process(context);
    _outputTrim.setGainDecibels(getParameterValue(_outputParam, 0.0f));
    _outputTrim.process(context);

    if (numChannels > 1) {
        juce::FloatVectorOperations::copy(buffer.getWritePointer(1), buffer.getReadPointer(0), numSamples);
    }

    _rmsLevelOutput.store(juce::Decibels::gainToDecibels(buffer.getRMSLevel(0, 0, numSamples), -60.0f),
                          std::memory_order_relaxed);
}

float ProfilerAudioProcessor::getRmsLevelInput() const noexcept {
    return _rmsLevelInput.load(std::memory_order_relaxed);
}

float ProfilerAudioProcessor::getRmsLevelOutput() const noexcept {
    return _rmsLevelOutput.load(std::memory_order_relaxed);
}

SignalChain::Layout ProfilerAudioProcessor::getChainLayout() const noexcept {
    return SignalChain::Layout::fromPacked(_chainLayoutPacked.load(std::memory_order_relaxed));
}

void ProfilerAudioProcessor::setChainLayout(const SignalChain::Layout& layout) {
    const auto valid = layout.isValid() ? layout : SignalChain::Layout{};
    const auto packed = valid.packed();
    _chainLayoutPacked.store(packed, std::memory_order_relaxed);
    if (_apvts.state.isValid()) {
        _apvts.state.setProperty("chainLayout", static_cast<int>(packed), nullptr);
    }
}

void ProfilerAudioProcessor::resetChainLayout() {
    setChainLayout({});
}

void ProfilerAudioProcessor::placeChainStage(SignalChain::Stage stage, int slot) {
    auto layout = getChainLayout();
    layout.place(stage, slot);
    setChainLayout(layout);
}

void ProfilerAudioProcessor::processGateStage(juce::dsp::ProcessContextReplacing<float>& context) {
    if (!_chain.isBypassed<NoiseGate>()) {
        _chain.get<NoiseGate>().process(context);
    }
}

void ProfilerAudioProcessor::processAmpStage(juce::AudioBuffer<float>& buffer,
                                             juce::dsp::ProcessContextReplacing<float>& context,
                                             int numSamples) {
    _chain.get<Gain>().process(context);

    {
        const juce::ScopedLock ampLock(_ampModelLock);
        if (_ampLoaded && _neuralAmp != nullptr && getParameterValue(_isAmpEnabledParam, 1.0f) > 0.5f) {
            auto* channel0Data = buffer.getWritePointer(0);

            for (int i = 0; i < numSamples; ++i) {
                float input = juce::jlimit(-1.0f, 1.0f, channel0Data[i]);
                float inputSample[] = {input};
                _neuralAmp->forward(inputSample);
                float output = _neuralAmp->getOutputs()[0];

                if (std::isnan(output) || std::isinf(output)) {
                    _neuralAmp->reset();
                    output = 0.0f;
                }

                channel0Data[i] = juce::jlimit(-1.0f, 1.0f, output);
            }
        }
    }

    _dcBlocker.process(context);
}

void ProfilerAudioProcessor::processCabStage(juce::dsp::ProcessContextReplacing<float>& context) {
    const bool cabEnabled = getParameterValue(_isCabEnabledParam, 1.0f) > 0.5f;
    if (!_irLoaded || !cabEnabled) {
        return;
    }

    _convolver.process(context);
    if (_cabLowCutSmoothed.isSmoothing()) {
        updateCabLowCutCoefficients();
    }
    _cabLowCut.process(context);
}

void ProfilerAudioProcessor::processEqStage(juce::dsp::ProcessContextReplacing<float>& context) {
    _eqChain.process(context);
}

void ProfilerAudioProcessor::processPedalStage(juce::AudioBuffer<float>& buffer,
                                               juce::dsp::ProcessContextReplacing<float>& context,
                                               int numSamples) {
    if (getParameterValue(_isPedalEnabledParam, 1.0f) <= 0.5f) {
        return;
    }

    const auto drive = juce::jmap(getParameterValue(_pedalDriveParam, 4.0f), 0.0f, 10.0f, 1.0f, 16.0f);
    const auto level = juce::Decibels::decibelsToGain(getParameterValue(_pedalLevelParam, 0.0f));
    const auto makeup = 1.0f / std::tanh(drive * 0.35f);
    auto* channel0Data = buffer.getWritePointer(0);
    for (int i = 0; i < numSamples; ++i) {
        channel0Data[i] = std::tanh(channel0Data[i] * drive) * makeup * level;
    }

    updatePedalToneCoefficients();
    _pedalToneFilter.process(context);
}

void ProfilerAudioProcessor::updatePedalToneCoefficients() {
    const auto sampleRate = getSampleRate();
    if (sampleRate <= 0.0) {
        return;
    }

    const auto tone = juce::jlimit(0.0f, 1.0f, getParameterValue(_pedalToneParam, 65.0f) / 100.0f);
    const auto hz = 400.0f * std::pow(30.0f, tone);
    *_pedalToneFilter.state = juce::dsp::IIR::ArrayCoefficients<float>::makeLowPass(
        sampleRate, juce::jlimit(400.0f, static_cast<float>(sampleRate * 0.45), hz));
}

void ProfilerAudioProcessor::updateEqCoefficients() {
    const auto sampleRate = getSampleRate();
    if (sampleRate <= 0.0) {
        return;
    }

    const auto nyquist = static_cast<float>(sampleRate * 0.45);

    auto freqFor = [nyquist](float hz) {
        return juce::jlimit(EqBands::minHz, nyquist, hz);
    };

    auto gainFor = [](float db) {
        return juce::Decibels::decibelsToGain(db);
    };

    *_eqChain.get<LowShelf>().state = juce::dsp::IIR::ArrayCoefficients<float>::makeLowShelf(
        sampleRate, freqFor(_eqFreqSmoothed[0].getCurrentValue()), SHELF_Q, gainFor(_eqGainSmoothed[0].getCurrentValue()));
    *_eqChain.get<Peak1>().state = juce::dsp::IIR::ArrayCoefficients<float>::makePeakFilter(
        sampleRate, freqFor(_eqFreqSmoothed[1].getCurrentValue()), PEAK_Q, gainFor(_eqGainSmoothed[1].getCurrentValue()));
    *_eqChain.get<Peak2>().state = juce::dsp::IIR::ArrayCoefficients<float>::makePeakFilter(
        sampleRate, freqFor(_eqFreqSmoothed[2].getCurrentValue()), PEAK_Q, gainFor(_eqGainSmoothed[2].getCurrentValue()));
    *_eqChain.get<Peak3>().state = juce::dsp::IIR::ArrayCoefficients<float>::makePeakFilter(
        sampleRate, freqFor(_eqFreqSmoothed[3].getCurrentValue()), PEAK_Q, gainFor(_eqGainSmoothed[3].getCurrentValue()));
    *_eqChain.get<Peak4>().state = juce::dsp::IIR::ArrayCoefficients<float>::makePeakFilter(
        sampleRate, freqFor(_eqFreqSmoothed[4].getCurrentValue()), PEAK_Q, gainFor(_eqGainSmoothed[4].getCurrentValue()));
    *_eqChain.get<HighShelf>().state = juce::dsp::IIR::ArrayCoefficients<float>::makeHighShelf(
        sampleRate, freqFor(_eqFreqSmoothed[5].getCurrentValue()), SHELF_Q, gainFor(_eqGainSmoothed[5].getCurrentValue()));
}

void ProfilerAudioProcessor::updateCabLowCutCoefficients() {
    *_cabLowCut.state = juce::dsp::IIR::ArrayCoefficients<float>::makeHighPass(
        getSampleRate(),
        juce::jlimit(20.0f, 250.0f, _cabLowCutSmoothed.getCurrentValue()),
        SHELF_Q);
}

//==============================================================================
bool ProfilerAudioProcessor::hasEditor() const {
#if defined(PROFILER_HEADLESS_TESTS) && PROFILER_HEADLESS_TESTS
    return false;
#else
    return true;
#endif
}

juce::AudioProcessorEditor* ProfilerAudioProcessor::createEditor() {
#if defined(PROFILER_HEADLESS_TESTS) && PROFILER_HEADLESS_TESTS
    return nullptr;
#else
    return new ProfilerAudioProcessorEditor(*this);
#endif
}

//==============================================================================
void ProfilerAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    _apvts.state.setProperty("chainLayout", static_cast<int>(_chainLayoutPacked.load(std::memory_order_relaxed)), nullptr);
    if (auto xml = _apvts.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void ProfilerAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    if (auto xml = getXmlFromBinary(data, sizeInBytes)) {
        if (xml->hasTagName(_apvts.state.getType())) {
            auto tree = juce::ValueTree::fromXml(*xml);
            const auto packed = static_cast<std::uint32_t>(static_cast<int>(
                tree.getProperty("chainLayout", static_cast<int>(SignalChain::defaultPacked))));
            _chainLayoutPacked.store(SignalChain::Layout::fromPacked(packed).packed(), std::memory_order_relaxed);
            _apvts.replaceState(std::move(tree));
        }
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout ProfilerAudioProcessor::createParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // ==============================================================================
    // Master parameters
    // ==============================================================================
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"master", 1}, "Master Volume", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 50.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"gain", 1}, "Gain", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"noise", 1}, "Noise Gate", juce::NormalisableRange<float>(0.0f, 60.0f, 0.1f), 10.0f));

    // Fix P0: Add missing Input and Output parameters
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"input", 1}, "Input Trim", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"output", 1}, "Output Trim", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f));

    // ==============================================================================
    // EQ parameters
    // ==============================================================================
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"depth", 1}, "Low Shelf", EqBands::gainRange(), EqBands::specs[0].defaultDb));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"bass", 1}, "EQ Band 1", EqBands::gainRange(), EqBands::specs[1].defaultDb));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"mid", 1}, "EQ Band 2", EqBands::gainRange(), EqBands::specs[2].defaultDb));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"highMid", 1}, "EQ Band 3", EqBands::gainRange(), EqBands::specs[3].defaultDb));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"treble", 1}, "EQ Band 4", EqBands::gainRange(), EqBands::specs[4].defaultDb));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"presence", 1}, "High Shelf", EqBands::gainRange(), EqBands::specs[5].defaultDb));

    for (const auto& band : EqBands::specs) {
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{band.freqId, 1},
            juce::String(band.label) + " Freq",
            EqBands::freqRange(),
            band.defaultHz));
    }

    // ==============================================================================
    // Other parameters
    // ==============================================================================
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"isMute", 1}, "Mute", false));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"isEqEnabled", 1}, "EQ", true));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"isGateEnabled", 1}, "Gate Enabled", true));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"isAmpEnabled", 1}, "Amp Enabled", true));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"isCabEnabled", 1}, "Cab Enabled", true));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"cabLowCut", 1}, "Cab Low Cut", juce::NormalisableRange<float>(20.0f, 250.0f, 1.0f), 80.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"isPedalEnabled", 1}, "Pedal Enabled", true));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"pedalDrive", 1}, "Pedal Drive", juce::NormalisableRange<float>(0.0f, 10.0f, 0.1f), 4.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"pedalTone", 1}, "Pedal Tone", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 65.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"pedalLevel", 1}, "Pedal Level", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new ProfilerAudioProcessor();
}

bool ProfilerAudioProcessor::loadIrFromMemory(const void* data, size_t size) {
    if (data == nullptr || size == 0) {
        return false;
    }

    _convolver.loadImpulseResponse(data,
                                   size,
                                   juce::dsp::Convolution::Stereo::no,
                                   juce::dsp::Convolution::Trim::yes,
                                   0);
    _irLoaded = true;
    _currentIRFile = juce::File{};
    return true;
}

bool ProfilerAudioProcessor::loadIRFile(const juce::File& file) {
    if (!file.existsAsFile()) {
        return false;
    }

    juce::MemoryBlock bytes;
    if (!file.loadFileAsData(bytes)) {
        return false;
    }

    const auto loaded = loadIrFromMemory(bytes.getData(), bytes.getSize());
    bytes.fillWith(0);
    if (!loaded) {
        return false;
    }

    _currentIRFile = file;
    return true;
}

bool ProfilerAudioProcessor::loadProtectedIr(const void* data, size_t size) {
    juce::MemoryBlock plain;
    if (!_spectra.decryptIr(data, size, plain)) {
        return false;
    }

    const auto loaded = loadIrFromMemory(plain.getData(), plain.getSize());
    plain.fillWith(0);
    return loaded;
}

void ProfilerAudioProcessor::unloadIRFile() {
    _irLoaded = false;
    _currentIRFile = juce::File{};
    _convolver.reset();
}

std::unique_ptr<RTNeural::Model<float>> ProfilerAudioProcessor::parseAmpModel(const void* data, size_t size) const {
    if (data == nullptr || size == 0) {
        return nullptr;
    }

    std::vector<char> bytes(size);
    std::memcpy(bytes.data(), data, size);
    std::unique_ptr<RTNeural::Model<float>> loadedAmp;
    try {
        const auto parsed = nlohmann::json::parse(std::string(bytes.data(), bytes.size()));
        loadedAmp = RTNeural::json_parser::parseJson<float>(parsed);
    } catch (const std::exception& e) {
        juce::Logger::writeToLog("RTNeural Load Error: " + juce::String(e.what()));
        loadedAmp.reset();
    }
    std::fill(bytes.begin(), bytes.end(), '\0');

    if (loadedAmp == nullptr) {
        return nullptr;
    }

    if (!isCompatibleAmpModel(*loadedAmp)) {
        juce::Logger::writeToLog("RTNeural Load Error: expected a mono-input model with at least one output.");
        return nullptr;
    }

    loadedAmp->reset();
    return loadedAmp;
}

bool ProfilerAudioProcessor::publishAmpModel(std::unique_ptr<RTNeural::Model<float>> model,
                                             const juce::File& sourceFile) {
    if (model == nullptr) {
        return false;
    }

    const juce::ScopedLock ampLock(_ampModelLock);
    _neuralAmp = std::move(model);
    _ampLoaded = true;
    _currentAmpFile = sourceFile;
    _ampFileLoaded = sourceFile.existsAsFile();
    return true;
}

bool ProfilerAudioProcessor::loadAmpFromMemory(const void* data, size_t size) {
    return publishAmpModel(parseAmpModel(data, size), {});
}

bool ProfilerAudioProcessor::loadAmpFile(const juce::File& file) {
    if (!file.existsAsFile()) {
        return false;
    }

    juce::MemoryBlock bytes;
    if (!file.loadFileAsData(bytes)) {
        return false;
    }

    auto loadedAmp = parseAmpModel(bytes.getData(), bytes.getSize());
    bytes.fillWith(0);
    return publishAmpModel(std::move(loadedAmp), file);
}

bool ProfilerAudioProcessor::loadProtectedAmp(const void* data, size_t size) {
    juce::MemoryBlock plain;
    if (!_spectra.decryptModel(data, size, plain)) {
        return false;
    }

    auto loadedAmp = parseAmpModel(plain.getData(), plain.getSize());
    plain.fillWith(0);
    return publishAmpModel(std::move(loadedAmp), {});
}

bool ProfilerAudioProcessor::isSpectraLoaded() const noexcept {
    return _spectra.isLoaded();
}

bool ProfilerAudioProcessor::unlockSpectraSession(const juce::String& accessToken) {
    return _spectra.unlock(accessToken);
}

void ProfilerAudioProcessor::lockSpectraSession() {
    _spectra.lock();
}

void ProfilerAudioProcessor::unloadAmpFile() {
    const juce::ScopedLock ampLock(_ampModelLock);
    _ampLoaded = false;
    _neuralAmp = nullptr;
    _ampFileLoaded = false;
    _currentAmpFile = juce::File{};
}

bool ProfilerAudioProcessor::isIRLoaded() const noexcept {
    return _irLoaded;
}

bool ProfilerAudioProcessor::isAmpFileLoaded() const noexcept {
    const juce::ScopedLock ampLock(_ampModelLock);
    return _ampFileLoaded;
}

juce::File ProfilerAudioProcessor::getCurrentIRFile() const {
    return _currentIRFile;
}

juce::File ProfilerAudioProcessor::getCurrentAmpFile() const {
    const juce::ScopedLock ampLock(_ampModelLock);
    return _currentAmpFile;
}

bool ProfilerAudioProcessor::applyProfile(int profileIndex, juce::String* errorMessage) {
    if (!_profileManager.applyProfile(profileIndex, errorMessage)) {
        return false;
    }

    const auto* profile = _profileManager.getProfile(profileIndex);
    const auto values = _profileManager.getProfileValues(profileIndex);
    applyProfileFileValues(values);

    _appliedProfileId = profile != nullptr ? profile->id : juce::String{};
    return true;
}

void ProfilerAudioProcessor::syncLoadedFilesWithCurrentProfile() {
    const auto currentProfileIndex = _profileManager.getCurrentProfileIndex();
    const auto* profile = _profileManager.getProfile(currentProfileIndex);

    if (profile == nullptr) {
        unloadIRFile();
        unloadAmpFile();
        clearAppliedProfile();
        return;
    }

    applyProfileFileValues(_profileManager.getProfileValues(currentProfileIndex));
    _appliedProfileId = profile->id;
}

void ProfilerAudioProcessor::applyProfileFileValues(const juce::NamedValueSet& values) {
    if (const auto* irPath = values.getVarPointer("irPath")) {
        const auto irPathText = irPath->toString().trim();
        const auto irFile = juce::File(irPathText);
        if (irPathText.isEmpty()) {
            unloadIRFile();
        } else if (irFile.existsAsFile()) {
            loadIRFile(irFile);
        } else {
            unloadIRFile();
            _currentIRFile = irFile;
        }
    } else {
        unloadIRFile();
    }

    if (const auto* ampPath = values.getVarPointer("ampPath")) {
        const auto ampPathText = ampPath->toString().trim();
        const auto ampFile = juce::File(ampPathText);
        if (ampPathText.isEmpty()) {
            unloadAmpFile();
        } else if (ampFile.existsAsFile()) {
            loadAmpFile(ampFile);
        } else {
            unloadAmpFile();
            _currentAmpFile = ampFile;
        }
    } else {
        unloadAmpFile();
    }
}

juce::String ProfilerAudioProcessor::getAppliedProfileId() const {
    return _appliedProfileId;
}

void ProfilerAudioProcessor::clearAppliedProfile() {
    _appliedProfileId.clear();
}

ProfileManager& ProfilerAudioProcessor::getProfileManager() noexcept {
    return _profileManager;
}

const ProfileManager& ProfilerAudioProcessor::getProfileManager() const noexcept {
    return _profileManager;
}
