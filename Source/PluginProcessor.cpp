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
    SweepGenerator::generateLogSweep(sweepBuffer, sampleRate, 15.0f);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    convolver.prepare(spec);
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
        buffer.clear(i, 0, buffer.getNumSamples());

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
    if (ampLoaded)
    {
        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);

            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                float x = juce::jlimit(-1.0f, 1.0f, channelData[i]);
                float pos = (x + 1.0f) * 0.5f * (ampLUT.size() - 1);
                int idx = (int)pos;
                float frac = pos - idx;
                float y = ampLUT[idx];
                if (idx + 1 < ampLUT.size())
                    y = y * (1.0f - frac) + ampLUT[idx + 1] * frac;

                channelData[i] = y;
            }
        }
    }

    if (irLoaded)
    {
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        convolver.process(context);
    }

    // start the sweep if button pressed
    if (sweepRunning)
    {
        int numSamples = buffer.getNumSamples();
        int sweepSamples = sweepBuffer.getNumSamples();

        for (int i = 0; i < numSamples; ++i)
        {
            float s = 0.0f;

            if (sweepPos < sweepSamples)
            {
                s = sweepBuffer.getSample(0, sweepPos);
                sweepPos++;
            }
            else
            {
                sweepRunning = false; // sweep terminé
            }

            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                buffer.setSample(ch, i, s);
        }
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
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ProfilerAudioProcessor();
}

void ProfilerAudioProcessor::startSweep()
{
    // Génère un sweep log de 15 secondes de 20Hz à 20kHz avec fade
    SweepGenerator::generateLogSweep(sweepBuffer, getSampleRate(), 15.0f);

    // Réinitialise la position et lance le sweep
    sweepPos = 0;
    sweepRunning = true;
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
                convolver.loadImpulseResponse(file,
                    juce::dsp::Convolution::Stereo::yes,
                    juce::dsp::Convolution::Trim::no,
                    0);
                irLoaded = true;
            }
            delete chooser; // libère la mémoire après usage
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
    ampLUT.resize(lutSize, 0.0f);
    std::vector<int> counts(lutSize, 0);

    for (int i = 0; i < numSamples; ++i)
    {
        float x = juce::jlimit(-1.0f, 1.0f, diBuf.getSample(0, i));
        float y = ampBuf.getSample(0, i);
        int idx = int((x + 1.0f) * 0.5f * (lutSize - 1));
        ampLUT[idx] += y;
        counts[idx]++;
    }

    for (int i = 0; i < lutSize; ++i)
    {
        if (counts[i] > 0) ampLUT[i] /= counts[i];
    }

    ampLoaded = true;
}
