#include "PluginProcessor.h"

#include "Logging.h"
#include "PluginEditor.h"

//==============================================================================
ProfilerAudioProcessor::ProfilerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(
          BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
              ),
      _profileManager(_apvts)
#else
    : _profileManager(_apvts)
#endif
{
    LoggingConfig config = LoggingConfigLoader::loadFromFile(
        juce::File(".config/log_settings.json"));
    AppLogger::initialise(config);
    AppLogger::info(LogCategory::Init, "Plugin instance created");

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
    AppLogger::info(LogCategory::Init, "Plugin instance destroyed");
    AppLogger::shutdown();
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
int ProfilerAudioProcessor::getNumPrograms() {return 1;}
int ProfilerAudioProcessor::getCurrentProgram() { return 0; }
void ProfilerAudioProcessor::setCurrentProgram(int index) {}
const juce::String ProfilerAudioProcessor::getProgramName(int index) {return {};}
void ProfilerAudioProcessor::changeProgramName(int index, const juce::String& newName) {}

//==============================================================================
void ProfilerAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32)samplesPerBlock;
    spec.numChannels = (juce::uint32)getTotalNumOutputChannels();

    _convolver.prepare(spec);

    _chain.reset();
    _chain.prepare(spec);

    _chain.get<Gain>().setGainDecibels(_gainParam->load());
    _chain.get<Gain>().setRampDurationSeconds(0.05);

    _chain.get<NoiseGate>().setThreshold(_noiseParam->load() - 60.0f);
    _chain.get<NoiseGate>().setAttack(5.0f);
    _chain.get<NoiseGate>().setRelease(100.0f);
    _chain.get<NoiseGate>().setRatio(10.0f);

    _chain.get<MasterVolume>().setGainDecibels(_masterParam->load() / 100.0f);
    _chain.get<MasterVolume>().setRampDurationSeconds(0.05);

    const bool eqEnabled = _isEqEnabledParam->load() > 0.5f;

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
    
    // Si tu utilises l'oversampling, la fréquence change pour l'ampli !
    // Exemple : si le host est à 44.1kHz, l'oversampler x2 envoie du 88.2kHz à l'ampli.
    oversampler.initProcessing(samplesPerBlock);

    // MAYBE DELETE THIS, IT'S NOT USED
    _ampStage.prepare(static_cast<float>(sampleRate));

    //     spec.maximumBlockSize = samplesPerBlock;
    //     spec.numChannels = getTotalNumOutputChannels();
    if (_neuralAmp != nullptr) {
        _neuralAmp->reset();
        _ampLoaded = true;
    }
}

void ProfilerAudioProcessor::releaseResources() {
    _chain.reset();
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

    // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
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
void ProfilerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int totalNumInputChannels = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();

    // Handle any incoming MIDI messages (e.g., for parameter automation)
    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // updateFilterCoefficients();

    float inputFactor = juce::Decibels::decibelsToGain(_apvts.getRawParameterValue("input")->load());
    float outputFactor = juce::Decibels::decibelsToGain(_apvts.getRawParameterValue("output")->load());

    const bool eqEnabled = _isEqEnabledParam != nullptr && _isEqEnabledParam->load() > 0.5f;

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

    if (_ampLoaded && _neuralAmp != nullptr) {
        const int numChans = buffer.getNumChannels();
        const int numSamps = buffer.getNumSamples();

        auto* channel0Data = buffer.getWritePointer(0);

        for (int i = 0; i < numSamps; ++i) 
        {
            // 1. Protection entrée : on évite d'envoyer un signal trop fort qui ferait exploser le réseau
            float input = juce::jlimit(-1.0f, 1.0f, channel0Data[i]);

            // 2. Traitement par RTNeural
            float inputSample[] = { input };
            _neuralAmp->forward(inputSample);
            float output = _neuralAmp->getOutputs()[0];

            if (std::isnan(output) || std::isinf(output)) 
            {
                _neuralAmp->reset();
                output = 0.0f;
            }

            // 4. On applique le signal traité au buffer (avec une limite de sécurité à 1.0)
            channel0Data[i] = juce::jlimit(-1.0f, 1.0f, output);
        }

        // 5. Duplication stricte sur le canal droit (Stéréo)
        if (numChans > 1) {
            auto* channel1Data = buffer.getWritePointer(1);
            juce::FloatVectorOperations::copy(channel1Data, channel0Data, numSamps);
        }
    }

    juce::dsp::AudioBlock<float> postAmpBlock(buffer);
    juce::dsp::ProcessContextReplacing<float> postAmpContext(postAmpBlock);
    _dcBlocker.process(postAmpContext);

    // 4. Cabinet Simulation
    if (_irLoaded) {
        _convolver.process(context);
    }
}

void ProfilerAudioProcessor::updateEqCoefficients() {
    *_chain.get<Depth>().state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(
        getSampleRate(), DEPTH_FREQ, SHELF_Q, juce::Decibels::decibelsToGain(_depthParam->load()));

    *_chain.get<Bass>().state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        getSampleRate(), BASS_FREQ, PEAK_Q, juce::Decibels::decibelsToGain(_bassParam->load()));

    *_chain.get<Mid>().state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        getSampleRate(), MID_FREQ, PEAK_Q, juce::Decibels::decibelsToGain(_midParam->load()));

    *_chain.get<Treble>().state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        getSampleRate(), TREBLE_FREQ, PEAK_Q, juce::Decibels::decibelsToGain(_trebleParam->load()));

    *_chain.get<Presence>().state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        getSampleRate(), PRESENCE_FREQ, SHELF_Q, juce::Decibels::decibelsToGain(_presenceParam->load()));
}

//==============================================================================
bool ProfilerAudioProcessor::hasEditor() const {return true;}

juce::AudioProcessorEditor* ProfilerAudioProcessor::createEditor() {
    return new ProfilerAudioProcessorEditor(*this);
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

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout ProfilerAudioProcessor::createParameterLayout() {
    // Create parameter layout here and add parameters to it
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    //==============================================================================
    // Master parameters
    //==============================================================================

    // Master Volume (0 to 100 %)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"master", 1},
        "Master Volume",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        50.0f));

    // Input Gain (-12 to +12 dB)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"gain", 1},
        "Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
        0.0f));

    // Noise Gate Threshold (-12 to +12 dB)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"noise", 1},
        "Noise Gate",
        juce::NormalisableRange<float>(0.0f, 60.0f, 0.1f),
        10.0f));

    //==============================================================================
    // EQ parameters
    //==============================================================================

    // Bass Gain (-24 to +24 dB)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"bass", 1},
        "Bass",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
        0.0f));

    // Mid Gain (-24 to +24 dB)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"mid", 1},
        "Mid",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
        0.0f));

    // Treble Gain (-24 to +24 dB)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"treble", 1},
        "Treble",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
        0.0f));

    // Presence Gain (-24 to +24 dB)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"presence", 1},
        "Presence",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
        0.0f));

    // Depth Gain (-24 to +24 dB)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"depth", 1},
        "Depth",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
        0.0f));

    //==============================================================================
    // Other parameters (e.g. Mute)
    //==============================================================================

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"isMute", 1},
        "Mute",
        false));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"isEqEnabled", 1},
        "EQ",
        true));

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

// MAYBE DELETE THIS FUNCTION, IT'S NOT USED
bool ProfilerAudioProcessor::loadAmpFile(const juce::File& file) {}

// MAYBE DELETE THIS FUNCTION, IT'S NOT USED
void ProfilerAudioProcessor::unloadAmpFile() {
    _ampFileLoaded = false;
    _currentAmpFile = juce::File{};
}

// MAYBE DELETE THIS FUNCTION, IT'S NOT USED
bool ProfilerAudioProcessor::isIRLoaded() const noexcept {
    return _irLoaded;
}

// MAYBE DELETE THIS FUNCTION, IT'S NOT USED
bool ProfilerAudioProcessor::isAmpFileLoaded() const noexcept {
    return _ampFileLoaded;
}

// MAYBE DELETE THIS FUNCTION, IT'S NOT USED
juce::File ProfilerAudioProcessor::getCurrentIRFile() const {
    return _currentIRFile;
}

// MAYBE DELETE THIS FUNCTION, IT'S NOT USED
juce::File ProfilerAudioProcessor::getCurrentAmpFile() const {
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

void ProfilerAudioProcessor::loadJsonFile() {
    auto chooser = new juce::FileChooser("Select a config file", {}, "*.json");
    chooser->launchAsync(juce::FileBrowserComponent::openMode |
                         juce::FileBrowserComponent::canSelectFiles,
                         [this, chooser](const juce::FileChooser& fc) {
                             auto file = fc.getResult();
                             if (file.existsAsFile()) {
                                 _ampLoaded = false;
                                 std::ifstream jsonStream(file.getFullPathName().toStdString());
                                 
                                 if (jsonStream.is_open()) {
                                     try {
                                         _neuralAmp = RTNeural::json_parser::parseJson<float>(jsonStream);
                                         
                                         if (_neuralAmp != nullptr) {
                                            _neuralAmp->reset();
                                             _ampLoaded = true;
                                         }
                                     }
                                     catch (const std::exception& e) {
                                         juce::Logger::writeToLog("RTNeural Load Error: " + juce::String(e.what()));
                                         _ampLoaded = false;
                                         _neuralAmp = nullptr;
                                     }
                                 }
                             }
                             delete chooser;
                         });
}

void ProfilerAudioProcessor::startAmpProfiling() {
    // _ampProfiling.generateAndSaveGainSignal();
    // _ampProfiling.generateSaturationProbe();
}

void ProfilerAudioProcessor::startGainAnalysis() {
    _ampProfiling.startGainAnalysis(_ampStage);
}

ProfileManager& ProfilerAudioProcessor::getProfileManager() noexcept {
    return _profileManager;
}

const ProfileManager& ProfilerAudioProcessor::getProfileManager() const noexcept {
    return _profileManager;
}
