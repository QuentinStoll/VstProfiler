#include "PluginProcessor.h"

#include <common/TracyColor.hpp>
#include <fstream>
#include <utility>

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
    _chain.setBypassed<NoiseGate>(getParameterValue(_isGateEnabledParam, 1.0f) <= 0.5f
                                 || getParameterValue(_noiseParam, 0.0f) <= 0.0f);

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
        eqSmoothing = eqSmoothing
                      || _eqGainSmoothed[index].isSmoothing()
                      || _eqFreqSmoothed[index].isSmoothing();
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
    _chain.process(context);

    {
        const juce::ScopedLock ampLock(_ampModelLock);
        if (_ampLoaded && _neuralAmp != nullptr && getParameterValue(_isAmpEnabledParam, 1.0f) > 0.5f) {
            auto* channel0Data = buffer.getWritePointer(0);

            for (int i = 0; i < numSamples; ++i) {
                // 1. Protection entrée : on évite d'envoyer un signal trop fort qui ferait exploser le réseau
                float input = juce::jlimit(-1.0f, 1.0f, channel0Data[i]);

                // 2. Traitement par RTNeural
                float inputSample[] = {input};
                _neuralAmp->forward(inputSample);
                float output = _neuralAmp->getOutputs()[0];

                if (std::isnan(output) || std::isinf(output)) {
                    _neuralAmp->reset();
                    output = 0.0f;
                }

                // 4. On applique le signal traité au buffer (avec une limite de sécurité à 1.0)
                channel0Data[i] = juce::jlimit(-1.0f, 1.0f, output);
            }
        }
    }

    juce::dsp::AudioBlock<float> postAmpBlock(buffer);
    auto postAmpMono = postAmpBlock.getSingleChannelBlock(0);
    juce::dsp::ProcessContextReplacing<float> postAmpContext(postAmpMono);
    _dcBlocker.process(postAmpContext);

    // 4. Cabinet Simulation
    const bool cabEnabled = getParameterValue(_isCabEnabledParam, 1.0f) > 0.5f;
    _cabLowCutSmoothed.setTargetValue(getParameterValue(_cabLowCutParam, 80.0f));
    if (_irLoaded && cabEnabled) {
        _convolver.process(postAmpContext);
        if (_cabLowCutSmoothed.isSmoothing()) {
            updateCabLowCutCoefficients();
        }
        _cabLowCut.process(postAmpContext);
    }
    _cabLowCutSmoothed.skip(numSamples);

    _eqChain.process(postAmpContext);
    for (auto& smoothed : _eqGainSmoothed) {
        smoothed.skip(numSamples);
    }
    for (auto& smoothed : _eqFreqSmoothed) {
        smoothed.skip(numSamples);
    }

    _masterVolume.setGainLinear(getMasterGainLinear(getParameterValue(_masterParam, 50.0f)));
    _masterVolume.process(postAmpContext);
    _outputTrim.setGainDecibels(getParameterValue(_outputParam, 0.0f));
    _outputTrim.process(postAmpContext);

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
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    if (auto xml = _apvts.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void ProfilerAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(_apvts.state.getType()))
            _apvts.replaceState(juce::ValueTree::fromXml(*xml));
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

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new ProfilerAudioProcessor();
}

bool ProfilerAudioProcessor::loadIRFile(const juce::File& file) {
    if (!file.existsAsFile()) {
        return false;
    }

    _convolver.loadImpulseResponse(file,
                                   juce::dsp::Convolution::Stereo::no,
                                   juce::dsp::Convolution::Trim::yes,
                                   0);
    _irLoaded = true;
    _currentIRFile = file;
    return true;
}

void ProfilerAudioProcessor::unloadIRFile() {
    _irLoaded = false;
    _currentIRFile = juce::File{};
    _convolver.reset();
}

bool ProfilerAudioProcessor::loadAmpFile(const juce::File& file) {
    if (!file.existsAsFile()) {
        return false;
    }

    std::unique_ptr<RTNeural::Model<float>> loadedAmp;
    std::ifstream jsonStream(file.getFullPathName().toStdString());
    if (!jsonStream.is_open()) {
        return false;
    }

    try {
        loadedAmp = RTNeural::json_parser::parseJson<float>(jsonStream);
    } catch (const std::exception& e) {
        juce::Logger::writeToLog("RTNeural Load Error: " + juce::String(e.what()));
        return false;
    }

    if (loadedAmp == nullptr) {
        return false;
    }

    if (!isCompatibleAmpModel(*loadedAmp)) {
        juce::Logger::writeToLog("RTNeural Load Error: expected a mono-input model with at least one output.");
        return false;
    }

    loadedAmp->reset();

    {
        const juce::ScopedLock ampLock(_ampModelLock);
        _neuralAmp = std::move(loadedAmp);
        _ampLoaded = true;
        _currentAmpFile = file;
        _ampFileLoaded = true;
    }

    return true;
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
