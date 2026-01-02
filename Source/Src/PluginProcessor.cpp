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
}

ProfilerAudioProcessor::~ProfilerAudioProcessor()
{
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
    _mainProcessor.prepare(spec);
    _mainProcessor.reset();

    oversampler.initProcessing(samplesPerBlock);
    
    _ampStage.prepare(sampleRate);


//     spec.maximumBlockSize = samplesPerBlock;
//     spec.numChannels = getTotalNumOutputChannels();

    
}


void ProfilerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
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

void ProfilerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();


    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
	// Update filter coefficients based on current parameter values
	updateFilterCoefficients();

	// Retrieve gain parameters
	float inputGain = _apvts.getRawParameterValue("input")->load();
	float outputGain = _apvts.getRawParameterValue("output")->load();

	// Convert dB to linear gain
    float inputFactor = juce::Decibels::decibelsToGain(inputGain);
	float outputFactor = juce::Decibels::decibelsToGain(outputGain);

	// Set gain values in the main processor chain
    _mainProcessor.get<0>().setGainLinear(inputFactor);
	_mainProcessor.get<5>().setGainLinear(outputFactor);

	// Create audio block and process context
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);

	// Process the audio through the main processor chain
    _mainProcessor.process(context);

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);

        // ..do something to the data...
    }

    if (_ampLoaded)
    {
        // A. Passage à une fréquence plus haute (ex: 44.1kHz -> 176.4kHz)
        juce::dsp::AudioBlock<float> oversampledBlock = oversampler.processSamplesUp(block);

        for (int ch = 0; ch < (int)oversampledBlock.getNumChannels(); ++ch)
        {
            auto* data = oversampledBlock.getChannelPointer(ch);

            for (int i = 0; i < (int)oversampledBlock.getNumSamples(); ++i)
            {
                // On traite chaque échantillon à haute fréquence
                data[i] = _ampStage.processSample(data[i]);
            }
        }

        // B. Retour à la fréquence d'origine (Filtre passe-bas + Downsampling)
        oversampler.processSamplesDown(block);
    }

    if (_irLoaded)
    {
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        _convolver.process(context);
    }

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
}

void ProfilerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
void ProfilerAudioProcessor::updateFilterCoefficients()
{
	// Retrieve parameter values
	float gateThreshold = _apvts.getRawParameterValue("gate")->load();
	float bassGain = _apvts.getRawParameterValue("bass")->load();
	float midGain = _apvts.getRawParameterValue("middle")->load();
    float trebleGain = _apvts.getRawParameterValue("treble")->load();

	// Get the current sample rate
	double sampleRate = getSampleRate();

	// Update gate settings
	auto& gate = _mainProcessor.get<1>();
	gate.setThreshold(gateThreshold);
	gate.setAttack(10.0f);
	gate.setRelease(50.0f);
	gate.setRatio(4.0f);

	// Update EQ filter coefficients
	// Bass - Low Shelf
	_mainProcessor.get<2>().coefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
        sampleRate,
        100.0f,
        0.707f,
        juce::Decibels::decibelsToGain(bassGain)
	);

	// Mid - Peak Filter
    _mainProcessor.get<3>().coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        sampleRate,
        1000.0f,
        1.0f,
        juce::Decibels::decibelsToGain(midGain)
    );

	// Treble - High Shelf
    _mainProcessor.get<4>().coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        sampleRate,
        5000.0f,
        0.707f,
        juce::Decibels::decibelsToGain(trebleGain)
	);
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout ProfilerAudioProcessor::createParameterLayout()
{
    // Create parameter layout here and add parameters to it
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
	// Add input gain parameter
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "input", 1 },
        "Input", 
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
        0.0f
    ));

	// Add gate threshold parameter
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "gate", 1 },
        "Gate",
        juce::NormalisableRange<float>(-60.0f, 10.0f, 0.1f),
        -40.0f
    ));

	// Add EQ parameters
	// Bass
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "bass", 1 },
        "Bass",
        juce::NormalisableRange<float>(-15.0f, 15.0f, 0.1f),
        0.0f
    ));

	// Middle
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "middle", 1 },
        "Middle",
        juce::NormalisableRange<float>(-15.0f, 15.0f, 0.1f),
        0.0f
    ));

	// Treble
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "treble", 1 },
        "Treble",
        juce::NormalisableRange<float>(-15.0f, 15.0f, 0.1f),
        0.0f
    ));

	// Add output gain parameter
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "output", 1 },
        "Output",
        juce::NormalisableRange<float>(-48.0f, 12.0f, 0.1f),
        -6.0f
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
