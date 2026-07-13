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
    Log::LogConfig config = Log::LogConfig::fromFile((juce::File)("/home/krt/dev/EIP/VstProfiler/.config/log_settings.json"));
    Log::Logger& logger = Log::LogRegistry::create(config.name, config);

    _masterParam = _apvts.getRawParameterValue("master");
    _gainParam = _apvts.getRawParameterValue("gain");
    _noiseParam = _apvts.getRawParameterValue("noise");

    _depthParam = _apvts.getRawParameterValue("depth");
    _bassParam = _apvts.getRawParameterValue("bass");
    _midParam = _apvts.getRawParameterValue("mid");
    _trebleParam = _apvts.getRawParameterValue("treble");
    _presenceParam = _apvts.getRawParameterValue("presence");

    _isMuteParam = _apvts.getRawParameterValue("isMute");
    _isEqEnabledParam = _apvts.getRawParameterValue("isEqEnabled");
}

ProfilerAudioProcessor::~ProfilerAudioProcessor() {
    Log::LogRegistry::shutdownAll();
}

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
    spec.numChannels = (juce::uint32)getTotalNumOutputChannels();

    _convolver.prepare(spec);

    _chain.reset();
    _chain.prepare(spec);

    _chain.get<Gain>().setGainDecibels(getParameterValue(_gainParam, 0.0f));
    _chain.get<Gain>().setRampDurationSeconds(0.05);

    _masterVolume.prepare(spec);
    _masterVolume.setRampDurationSeconds(0.05);
    _masterVolume.setGainLinear(getMasterGainLinear(getParameterValue(_masterParam, 50.0f)));

    _chain.get<NoiseGate>().setThreshold(getParameterValue(_noiseParam, 10.0f) - 60.0f);
    _chain.get<NoiseGate>().setAttack(5.0f);
    _chain.get<NoiseGate>().setRelease(100.0f);
    _chain.get<NoiseGate>().setRatio(10.0f);
    _chain.setBypassed<NoiseGate>(getParameterValue(_noiseParam, 0.0f) <= 0.0f);

    const bool eqEnabled = getParameterValue(_isEqEnabledParam, 1.0f) > 0.5f;

    _chain.setBypassed<Depth>(!eqEnabled);
    _chain.setBypassed<Bass>(!eqEnabled);
    _chain.setBypassed<Mid>(!eqEnabled);
    _chain.setBypassed<Treble>(!eqEnabled);
    _chain.setBypassed<Presence>(!eqEnabled);

    *_chain.get<Depth>().state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, DEPTH_FREQ, SHELF_Q, 1.0f);
    *_chain.get<Bass>().state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, BASS_FREQ, PEAK_Q, 1.0f);
    *_chain.get<Mid>().state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, MID_FREQ, PEAK_Q, 1.0f);
    *_chain.get<Treble>().state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, TREBLE_FREQ, PEAK_Q, 1.0f);
    *_chain.get<Presence>().state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, PRESENCE_FREQ, SHELF_Q, 1.0f);

    *_dcBlocker.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 35.0f);
    _dcBlocker.prepare(spec);
    _dcBlocker.reset();

    oversampler.initProcessing(samplesPerBlock);

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
    _masterVolume.reset();
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
    Log::LogRegistry::get("MainLogger").warn(Log::LogCategory::Other, "test warning");
    ZoneScopedNC("test2", tracy::Color::Purple)

    juce::ScopedNoDenormals noDenormals;

    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();
    if (numChannels <= 0 || numSamples <= 0) {
        return;
    }

    // Handle Mute
    if (getParameterValue(_isMuteParam, 0.0f) > 0.5f) {
        buffer.clear();
        return;
    }

    // Handle any incoming MIDI messages (e.g., for parameter automation)
    for (auto i = getTotalNumInputChannels(); i < juce::jmin(getTotalNumOutputChannels(), numChannels); ++i)
        buffer.clear(i, 0, numSamples);

    _chain.get<Gain>().setGainDecibels(getParameterValue(_gainParam, 0.0f));
    const auto noiseGateValue = getParameterValue(_noiseParam, 0.0f);
    _chain.get<NoiseGate>().setThreshold(noiseGateValue - 60.0f);
    // A zero control value is a true bypass, rather than a -60 dB gate.
    _chain.setBypassed<NoiseGate>(noiseGateValue <= 0.0f);

    const bool eqEnabled = getParameterValue(_isEqEnabledParam, 1.0f) > 0.5f;

    _chain.setBypassed<Depth>(!eqEnabled);
    _chain.setBypassed<Bass>(!eqEnabled);
    _chain.setBypassed<Mid>(!eqEnabled);
    _chain.setBypassed<Treble>(!eqEnabled);
    _chain.setBypassed<Presence>(!eqEnabled);

    if (eqEnabled) {
        updateEqCoefficients();
    }

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    _chain.process(context);

    {
        const juce::ScopedLock ampLock(_ampModelLock);
        if (_ampLoaded && _neuralAmp != nullptr) {
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

    // The amp is intentionally mono; duplicate its completed mono signal to
    // the stereo output so it is audible in both headphones/speakers.
    if (numChannels > 1) {
        juce::FloatVectorOperations::copy(buffer.getWritePointer(1), buffer.getReadPointer(0), numSamples);
    }

    juce::dsp::AudioBlock<float> postAmpBlock(buffer);
    juce::dsp::ProcessContextReplacing<float> postAmpContext(postAmpBlock);
    _dcBlocker.process(postAmpContext);

    // 4. Cabinet Simulation
    if (_irLoaded) {
        _convolver.process(postAmpContext);
    }

    _masterVolume.setGainLinear(getMasterGainLinear(getParameterValue(_masterParam, 50.0f)));
    _masterVolume.process(postAmpContext);
    _rmsLevelOutput.store(juce::Decibels::gainToDecibels(buffer.getRMSLevel(0, 0, numSamples), -60.0f),
                          std::memory_order_relaxed);
}

float ProfilerAudioProcessor::getRmsLevelOutput() const noexcept {
    return _rmsLevelOutput.load(std::memory_order_relaxed);
}

void ProfilerAudioProcessor::updateEqCoefficients() {
    *_chain.get<Depth>().state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(
        getSampleRate(), DEPTH_FREQ, SHELF_Q, juce::Decibels::decibelsToGain(getParameterValue(_depthParam, 0.0f)));

    *_chain.get<Bass>().state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        getSampleRate(), BASS_FREQ, PEAK_Q, juce::Decibels::decibelsToGain(getParameterValue(_bassParam, 0.0f)));

    *_chain.get<Mid>().state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        getSampleRate(), MID_FREQ, PEAK_Q, juce::Decibels::decibelsToGain(getParameterValue(_midParam, 0.0f)));

    *_chain.get<Treble>().state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        getSampleRate(), TREBLE_FREQ, PEAK_Q, juce::Decibels::decibelsToGain(getParameterValue(_trebleParam, 0.0f)));

    *_chain.get<Presence>().state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        getSampleRate(), PRESENCE_FREQ, SHELF_Q, juce::Decibels::decibelsToGain(getParameterValue(_presenceParam, 0.0f)));
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
        juce::ParameterID{"bass", 1}, "Bass", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"mid", 1}, "Mid", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"treble", 1}, "Treble", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"presence", 1}, "Presence", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"depth", 1}, "Depth", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f));

    // ==============================================================================
    // Other parameters
    // ==============================================================================
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"isMute", 1}, "Mute", false));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"isEqEnabled", 1}, "EQ", true));

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
                                   juce::dsp::Convolution::Stereo::yes,
                                   juce::dsp::Convolution::Trim::no,
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
