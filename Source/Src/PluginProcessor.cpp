#include "Logging.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ProfilerAudioProcessor::ProfilerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    LoggingConfig config= LoggingConfigLoader::loadFromFile(juce::File(".config/log_settings.json"));
    AppLogger::initialise();
    AppLogger::info(LogCategory::INIT, "Plugin instance created");

    _masterParam = _apvts.getRawParameterValue("master");
    _gainParam = _apvts.getRawParameterValue("gain");
    _noiseParam = _apvts.getRawParameterValue("noise");

	_depthParam = _apvts.getRawParameterValue("depth");
	_bassParam = _apvts.getRawParameterValue("bass");
	_midParam = _apvts.getRawParameterValue("mid");
	_trebleParam = _apvts.getRawParameterValue("treble");
	_presenceParam = _apvts.getRawParameterValue("presence");
}

ProfilerAudioProcessor::~ProfilerAudioProcessor() {
    AppLogger::info(LogCategory::INIT, "Plugin instance destroyed");
    AppLogger::shutdown();
}

//==============================================================================
const juce::String ProfilerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ProfilerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ProfilerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ProfilerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ProfilerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ProfilerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ProfilerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ProfilerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ProfilerAudioProcessor::getProgramName (int index)
{
    return {};
}

void ProfilerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ProfilerAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
  

	// Prepare the main processor chain
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

	_chain.get<Depth>().coefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, DEPTH_FREQ, SHELF_Q, 1.0f);
	_chain.get<Bass>().coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, BASS_FREQ, PEAK_Q, 1.0f);
	_chain.get<Mid>().coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, MID_FREQ, PEAK_Q, 1.0f);
	_chain.get<Treble>().coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, TREBLE_FREQ, PEAK_Q, 1.0f);
	_chain.get<Presence>().coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, PRESENCE_FREQ, SHELF_Q, 1.0f);

    oversampler.initProcessing(samplesPerBlock);
    
    _ampStage.prepare(sampleRate);

//     spec.maximumBlockSize = samplesPerBlock;
//     spec.numChannels = getTotalNumOutputChannels();
}


void ProfilerAudioProcessor::releaseResources()
{
	_chain.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ProfilerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

/**
    Performs the real-time audio processing. 
    This implementation handles gain scaling, a main DSP chain, 
    an oversampled non-linear amp stage, and an IR convolution stage.
*/
void ProfilerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // 1. Prepare Buffer & Parameters
    const int numSamples = buffer.getNumSamples();
    const int totalNumInputChannels = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, numSamples);

	_chain.get<Gain>().setGainDecibels(_gainParam->load());
    _chain.get<NoiseGate>().setThreshold(_noiseParam->load() - 60.0f);
	_chain.get<MasterVolume>().setGainLinear(_masterParam->load() / 100.0f);

	updateEqCoefficients();

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    _chain.process(context);

    // 3. Amp Simulation Stage (Non-linear processing with oversampling)
    if (_ampLoaded)
    {
        // Upsample to reduce aliasing distortion
        auto oversampledBlock = oversampler.processSamplesUp(block);
        
        const int numChans = (int)oversampledBlock.getNumChannels();
        const int numSamps = (int)oversampledBlock.getNumSamples();

        for (int ch = 0; ch < numChans; ++ch)
        {
            auto* data = oversampledBlock.getChannelPointer(ch);
            for (int i = 0; i < numSamps; ++i)
            {
                data[i] = _ampStage.processSample(data[i]);
            }
        }

        // Downsample back to the host's sample rate
        oversampler.processSamplesDown(block);
    }

    // 4. Cabinet Simulation (Convolution / IR)
    if (_irLoaded)
    {
        _convolver.process(context);
    }
}

void ProfilerAudioProcessor::updateEqCoefficients()
{
    _chain.get<Depth>().coefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
		getSampleRate(), DEPTH_FREQ, SHELF_Q, juce::Decibels::decibelsToGain(_depthParam->load()));

    _chain.get<Bass>().coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        getSampleRate(), BASS_FREQ, PEAK_Q, juce::Decibels::decibelsToGain(_bassParam->load()));

    _chain.get<Mid>().coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        getSampleRate(), MID_FREQ, PEAK_Q, juce::Decibels::decibelsToGain(_midParam->load()));

    _chain.get<Treble>().coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        getSampleRate(), TREBLE_FREQ, PEAK_Q, juce::Decibels::decibelsToGain(_trebleParam->load()));

	_chain.get<Presence>().coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
		getSampleRate(), PRESENCE_FREQ, SHELF_Q, juce::Decibels::decibelsToGain(_presenceParam->load()));
}

//==============================================================================
bool ProfilerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ProfilerAudioProcessor::createEditor()
{
    return new ProfilerAudioProcessorEditor (*this);
}

//==============================================================================
void ProfilerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    if (auto xml = _apvts.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void ProfilerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(_apvts.state.getType()))
            _apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout ProfilerAudioProcessor::createParameterLayout()
{
    // Create parameter layout here and add parameters to it
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
//==============================================================================
// Master parameters
//==============================================================================

	// Master Volume (0 to 100 %)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "master", 1 },
        "Master Volume",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
		50.0f
	));

	// Input Gain (-12 to +12 dB)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "gain", 1 },
        "Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
        0.0f
    ));

	// Noise Gate Threshold (-12 to +12 dB)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "noise", 1 },
        "Noise Gate",
        juce::NormalisableRange<float>(0.0f, 60.0f, 0.1f),
        10.0f
    ));

//==============================================================================
// EQ parameters
//==============================================================================

	// Bass Gain (-24 to +24 dB)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "bass", 1 },
        "Bass",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
        0.0f
    ));

	// Mid Gain (-24 to +24 dB)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "mid", 1 },
        "Mid",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
        0.0f
    ));

	// Treble Gain (-24 to +24 dB)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "treble", 1 },
        "Treble",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
        0.0f
    ));

	// Presence Gain (-24 to +24 dB)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "presence", 1 },
        "Presence",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
        0.0f
    ));

	// Depth Gain (-24 to +24 dB)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "depth", 1 },
        "Depth",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
        0.0f
    ));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ProfilerAudioProcessor();
}

void ProfilerAudioProcessor::loadIRFile()
{
    auto chooser = new juce::FileChooser("Select an IR file", {}, "*.wav");
    chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this, chooser](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file.existsAsFile())
            {
                _convolver.loadImpulseResponse(file,
                    juce::dsp::Convolution::Stereo::yes,
                    juce::dsp::Convolution::Trim::no,
                    0);
                _irLoaded = true;
            }
			delete chooser; // clean up memory
        });

}

void ProfilerAudioProcessor::startAmpProfiling()
{
    // _ampProfiling.generateAndSaveGainSignal();
    _ampProfiling.generateSaturationProbe();
}

void ProfilerAudioProcessor::startGainAnalysis()
{
    _ampProfiling.startGainAnalysis(_ampStage);
}