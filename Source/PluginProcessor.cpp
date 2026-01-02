/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

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
    SweepGenerator::generateLogSweep(_sweepBuffer, sampleRate, 15.0f);

	// Prepare the main processor chain
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32)samplesPerBlock;
    spec.numChannels = (juce::uint32)getTotalNumOutputChannels();

    _convolver.prepare(spec);
    _mainProcessor.prepare(spec);
    _mainProcessor.reset();

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
    
    float gainDb = _apvts.getRawParameterValue("gain")->load();
    const float gain = juce::Decibels::decibelsToGain(gainDb);
    float volume = _apvts.getRawParameterValue("volume")->load();

	// Set gain
    _mainProcessor.get<0>().setGainLinear(gain);

	// Update gate threshold
	updateGateThreshold();

	// Update filter coefficients based on current parameter values
	updateFilterCoefficients();

	// Create audio block and process context
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);

	// Process the audio through the main processor chain
    _mainProcessor.process(context);

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            auto currentSample = channelData[sample];
            currentSample = _mainProcessor.get<2>().processSample(currentSample);
            currentSample = _mainProcessor.get<3>().processSample(currentSample);
            currentSample = _mainProcessor.get<4>().processSample(currentSample);
            channelData[sample] = currentSample;
        }
    }

    if (_ampLoaded)
    {
        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);

            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                float x = juce::jlimit(-1.0f, 1.0f, channelData[i]);
                float pos = (x + 1.0f) * 0.5f * (_ampLUT.size() - 1);
                int idx = (int)pos;
                float frac = pos - idx;
                float y = _ampLUT[idx];
                if (idx + 1 < _ampLUT.size())
                    y = y * (1.0f - frac) + _ampLUT[idx + 1] * frac;

                channelData[i] = y;
            }
        }
    }

    if (_irLoaded)
    {
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        _convolver.process(context);
    }

    // start the sweep if button pressed
    if (_sweepRunning)
    {
        int numSamples = buffer.getNumSamples();
        int sweepSamples = _sweepBuffer.getNumSamples();

        for (int i = 0; i < numSamples; ++i)
        {
            float s = 0.0f;

            if (_sweepPos < sweepSamples)
            {
                s = _sweepBuffer.getSample(0, _sweepPos);
                _sweepPos++;
            }
            else
            {
                _sweepRunning = false; // sweep terminé
            }

            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                buffer.setSample(ch, i, s);
        }
    }

    // Set output volume
    _mainProcessor.get<5>().setGainLinear(volume);
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
	//float gateThreshold = _apvts.getRawParameterValue("gate")->load();
	float bassGain = _apvts.getRawParameterValue("bass")->load();
	float midGain = _apvts.getRawParameterValue("mid")->load();
    float trebleGain = _apvts.getRawParameterValue("treble")->load();

	// Get the current sample rate
	double sampleRate = getSampleRate();

	// Update EQ filter coefficients
	// Bass
	_mainProcessor.get<2>().coefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
        sampleRate,
        100.0f,
        0.7f,
        juce::Decibels::decibelsToGain(bassGain)
	);

	// Mid
    _mainProcessor.get<3>().coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        sampleRate,
        500.0f,
        0.7f,
        juce::Decibels::decibelsToGain(midGain)
    );

	// Treble
    _mainProcessor.get<4>().coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        sampleRate,
        5000.0f,
        0.7f,
        juce::Decibels::decibelsToGain(trebleGain)
	);
}

//==============================================================================
void ProfilerAudioProcessor::updateGateThreshold()
{
	// Retrieve gate parameter value
    float gateValue = _apvts.getRawParameterValue("gate")->load();
    
	// Map gateValue (0.0 to 10.0) to threshold in dB (-100 dB to -10 dB)
    float thresholdInDb = juce::jmap(gateValue, 0.0f, 10.0f, -100.0f, -10.0f);
    auto& gate = _mainProcessor.get<1>();
    
	// Set the threshold
	gate.setThreshold(thresholdInDb);

	// Set other gate parameters as needed
	gate.setAttack(5.0f);
	gate.setRelease(100.0f);
	gate.setRatio(10.0f);
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout ProfilerAudioProcessor::createParameterLayout()
{
    // Create parameter layout here and add parameters to it
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
	// Add input gain parameter
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "gain", 1 },
        "Gain", 
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.01f),
        0.0f
    ));

	// Add gate threshold parameter
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "gate", 1 },
        "Gate",
        juce::NormalisableRange<float>(0.0f, 10.0f, 0.01f),
        0.0f
    ));

	// Add EQ parameters
	// Bass
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "bass", 1 },
        "Bass",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.01f),
        0.0f
    ));

	// Middle
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "mid", 1 },
        "Mid",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.01f),
        0.0f
    ));

	// Treble
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "treble", 1 },
        "Treble",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.01f),
        0.0f
    ));

	// Add output gain parameter
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "volume", 1 },
        "Volume",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        1.0f
    ));
    
    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ProfilerAudioProcessor();
}

void ProfilerAudioProcessor::startSweep()
{
    SweepGenerator::generateLogSweep(_sweepBuffer, getSampleRate(), 15.0f);

	// Initialise sweep pos
    _sweepPos = 0;
    _sweepRunning = true;
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

// Fonction pour charger deux WAV et construire la LUT
void ProfilerAudioProcessor::loadAmpProfile()
{
    auto chooserDI = new juce::FileChooser("Select DI guitar file", {}, "*.wav");
    chooserDI->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this, chooserDI](const juce::FileChooser& fcDI)
        {
            auto diFile = fcDI.getResult();
            if (diFile.existsAsFile())
            {
                // Ensuite on choisit le fichier ampli
                auto chooserAmp = new juce::FileChooser("Select Amp output file", {}, "*.wav");
                chooserAmp->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                    [this, chooserAmp, diFile](const juce::FileChooser& fcAmp)
                    {
                        auto ampFile = fcAmp.getResult();
                        if (ampFile.existsAsFile())
                        {
                            // Maintenant on peut générer la LUT
                            generateAmpLUT(diFile, ampFile);
                        }
                        delete chooserAmp;
                    });
            }
            delete chooserDI;
        });
}

// Fonction qui construit la LUT à partir de deux WAV
void ProfilerAudioProcessor::generateAmpLUT(const juce::File& diFile, const juce::File& ampFile)
{
    juce::AudioFormatManager fm;
    fm.registerBasicFormats();

    std::unique_ptr<juce::AudioFormatReader> readerDI(fm.createReaderFor(diFile));
    std::unique_ptr<juce::AudioFormatReader> readerAmp(fm.createReaderFor(ampFile));

    if (!readerDI || !readerAmp) return;

    int numSamples = (int)std::min(readerDI->lengthInSamples, readerAmp->lengthInSamples);

    juce::AudioBuffer<float> diBuf(1, numSamples);
    juce::AudioBuffer<float> ampBuf(1, numSamples);

    readerDI->read(&diBuf, 0, numSamples, 0, true, false);
    readerAmp->read(&ampBuf, 0, numSamples, 0, true, false);

    int lutSize = 4096;
    _ampLUT.resize(lutSize, 0.0f);
    std::vector<int> counts(lutSize, 0);

    for (int i = 0; i < numSamples; ++i)
    {
        float x = juce::jlimit(-1.0f, 1.0f, diBuf.getSample(0, i));
        float y = ampBuf.getSample(0, i);
        int idx = int((x + 1.0f) * 0.5f * (lutSize - 1));
        _ampLUT[idx] += y;
        counts[idx]++;
    }

    for (int i = 0; i < lutSize; ++i)
    {
        if (counts[i] > 0) _ampLUT[i] /= counts[i];
    }

    _ampLoaded = true;
}
