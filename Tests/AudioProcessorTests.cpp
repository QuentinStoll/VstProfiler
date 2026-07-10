#include "TestRunner.h"

#include <JuceHeader.h>

#include <cmath>

#include "PluginProcessor.h"

namespace profiler_tests {
namespace {

class AudioProcessorUnitTests : public juce::UnitTest {
   public:
    AudioProcessorUnitTests()
        : juce::UnitTest("AudioProcessor", "Profiler") {}

    void runTest() override {
        runCase("audio processor metadata", [this] {
            testMetadata();
        });

        runCase("audio processor bus layouts", [this] {
            testBusLayouts();
        });

        runCase("audio processor parameter defaults", [this] {
            testParameterDefaults();
        });

        runCase("audio processor state round trip", [this] {
            testStateRoundTrip();
        });

        runCase("audio processor process block smoke", [this] {
            testProcessBlockSmoke();
        });

        runCase("audio processor mute clears buffer", [this] {
            testMuteClearsBuffer();
        });

        runCase("audio processor master zero silences buffer", [this] {
            testMasterZeroSilencesBuffer();
        });

        runCase("audio processor gain changes output level", [this] {
            testGainChangesOutputLevel();
        });

        runCase("audio processor silent input stays silent", [this] {
            testSilentInputStaysSilent();
        });

        runCase("audio processor handles prepare block sizes", [this] {
            testPrepareBlockSizes();
        });

        runCase("audio processor release and reprepare", [this] {
            testReleaseAndReprepare();
        });

        runCase("audio processor rejects missing asset files", [this] {
            testMissingAssetFiles();
        });
    }

   private:
    template <typename Function>
    void runCase(const juce::String& testCaseName, Function&& testCase) {
        if (requestedTestCase.isNotEmpty() && requestedTestCase != testCaseName) {
            return;
        }

        beginTest(testCaseName);
        testCase();
    }

    void expectClose(float actual, float expected, const juce::String& label) {
        expect(std::abs(actual - expected) <= 0.001f,
               label + " expected " + juce::String(expected) + " but got " + juce::String(actual));
    }

    float getParameterValue(ProfilerAudioProcessor& processor, const juce::String& parameterId) {
        const auto* value = processor._apvts.getRawParameterValue(parameterId);
        expect(value != nullptr, "Missing parameter: " + parameterId);
        return value != nullptr ? value->load() : 0.0f;
    }

    void setParameterValue(ProfilerAudioProcessor& processor,
                           const juce::String& parameterId,
                           float value) {
        auto* parameter = processor._apvts.getParameter(parameterId);
        expect(parameter != nullptr, "Missing parameter: " + parameterId);

        if (parameter != nullptr) {
            parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
        }
    }

    juce::AudioProcessor::BusesLayout makeLayout(const juce::AudioChannelSet& input,
                                                 const juce::AudioChannelSet& output) {
        juce::AudioProcessor::BusesLayout layout;
        layout.inputBuses.add(input);
        layout.outputBuses.add(output);
        return layout;
    }

    void prepareProcessor(ProfilerAudioProcessor& processor, double sampleRate, int blockSize) {
        processor.setRateAndBufferSizeDetails(sampleRate, blockSize);
        processor.prepareToPlay(sampleRate, blockSize);
    }

    void fillBuffer(juce::AudioBuffer<float>& buffer, float value) {
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
                buffer.setSample(channel, sample, value);
            }
        }
    }

    void fillSineBuffer(juce::AudioBuffer<float>& buffer,
                        double sampleRate,
                        float frequency,
                        float amplitude) {
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
                const auto phase = juce::MathConstants<double>::twoPi * frequency * sample / sampleRate;
                buffer.setSample(channel, sample, amplitude * static_cast<float>(std::sin(phase)));
            }
        }
    }

    bool allSamplesFinite(const juce::AudioBuffer<float>& buffer) {
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
            const auto* samples = buffer.getReadPointer(channel);
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
                if (!std::isfinite(samples[sample])) {
                    return false;
                }
            }
        }

        return true;
    }

    float getMaximumRmsLevel(const juce::AudioBuffer<float>& buffer) {
        float rms = 0.0f;
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
            rms = juce::jmax(rms, buffer.getRMSLevel(channel, 0, buffer.getNumSamples()));
        }

        return rms;
    }

    float processSineAndMeasureRms(ProfilerAudioProcessor& processor,
                                   int blockSize = 256,
                                   int blockCount = 32) {
        constexpr double sampleRate = 44100.0;
        juce::AudioBuffer<float> buffer(2, blockSize);
        juce::MidiBuffer midi;

        for (int block = 0; block < blockCount; ++block) {
            fillSineBuffer(buffer, sampleRate, 1000.0f, 0.25f);
            processor.processBlock(buffer, midi);
        }

        expect(allSamplesFinite(buffer), "Processed sine buffer should stay finite.");
        return getMaximumRmsLevel(buffer);
    }

    void testMetadata() {
        ProfilerAudioProcessor processor;

        expect(processor.getName() == "Profiler", "Processor should expose the plugin name.");
        expect(!processor.acceptsMidi(), "Profiler should not accept MIDI.");
        expect(!processor.producesMidi(), "Profiler should not produce MIDI.");
        expect(!processor.isMidiEffect(), "Profiler should not be a MIDI effect.");
        expect(processor.getTailLengthSeconds() == 0.0, "Profiler should not declare a tail.");
        expect(processor.getNumPrograms() == 1, "Profiler should expose one default program.");
        expect(processor.getCurrentProgram() == 0, "Profiler should start on program zero.");
        expect(processor.getProgramName(0).isEmpty(), "Default program should not have a custom name.");
    }

    void testBusLayouts() {
        ProfilerAudioProcessor processor;

        expect(processor.isBusesLayoutSupported(makeLayout(juce::AudioChannelSet::mono(),
                                                           juce::AudioChannelSet::mono())),
               "Mono input/output should be supported.");
        expect(processor.isBusesLayoutSupported(makeLayout(juce::AudioChannelSet::stereo(),
                                                           juce::AudioChannelSet::stereo())),
               "Stereo input/output should be supported.");
        expect(!processor.isBusesLayoutSupported(makeLayout(juce::AudioChannelSet::mono(),
                                                            juce::AudioChannelSet::stereo())),
               "Mismatched input/output layouts should be rejected.");
        expect(!processor.isBusesLayoutSupported(makeLayout(juce::AudioChannelSet::create5point1(),
                                                            juce::AudioChannelSet::create5point1())),
               "Surround layouts should be rejected.");
    }

    void testParameterDefaults() {
        ProfilerAudioProcessor processor;

        expect(processor.getParameters().size() == 12, "Unexpected processor parameter count.");
        expectClose(getParameterValue(processor, "master"), 50.0f, "master default");
        expectClose(getParameterValue(processor, "gain"), 0.0f, "gain default");
        expectClose(getParameterValue(processor, "noise"), 10.0f, "noise default");
        expectClose(getParameterValue(processor, "input"), 0.0f, "input default");
        expectClose(getParameterValue(processor, "output"), 0.0f, "output default");
        expectClose(getParameterValue(processor, "bass"), 0.0f, "bass default");
        expectClose(getParameterValue(processor, "mid"), 0.0f, "mid default");
        expectClose(getParameterValue(processor, "treble"), 0.0f, "treble default");
        expectClose(getParameterValue(processor, "presence"), 0.0f, "presence default");
        expectClose(getParameterValue(processor, "depth"), 0.0f, "depth default");
        expectClose(getParameterValue(processor, "isMute"), 0.0f, "isMute default");
        expectClose(getParameterValue(processor, "isEqEnabled"), 1.0f, "isEqEnabled default");
    }

    void testStateRoundTrip() {
        ProfilerAudioProcessor source;
        setParameterValue(source, "master", 25.0f);
        setParameterValue(source, "gain", -4.0f);
        setParameterValue(source, "bass", 6.0f);
        setParameterValue(source, "isMute", 1.0f);
        setParameterValue(source, "isEqEnabled", 0.0f);

        juce::MemoryBlock state;
        source.getStateInformation(state);
        expect(state.getSize() > 0, "Saved state should not be empty.");

        ProfilerAudioProcessor restored;
        restored.setStateInformation(state.getData(), static_cast<int>(state.getSize()));

        expectClose(getParameterValue(restored, "master"), 25.0f, "restored master");
        expectClose(getParameterValue(restored, "gain"), -4.0f, "restored gain");
        expectClose(getParameterValue(restored, "bass"), 6.0f, "restored bass");
        expectClose(getParameterValue(restored, "isMute"), 1.0f, "restored isMute");
        expectClose(getParameterValue(restored, "isEqEnabled"), 0.0f, "restored isEqEnabled");
    }

    void testProcessBlockSmoke() {
        ProfilerAudioProcessor processor;
        prepareProcessor(processor, 44100.0, 128);

        juce::AudioBuffer<float> buffer(2, 128);
        fillBuffer(buffer, 0.1f);
        juce::MidiBuffer midi;

        processor.processBlock(buffer, midi);

        expect(allSamplesFinite(buffer), "Processed samples should stay finite.");
        expect(buffer.getMagnitude(0, buffer.getNumSamples()) > 0.0f,
               "Processor should leave an audible signal for a non-muted input.");
        processor.releaseResources();
    }

    void testMuteClearsBuffer() {
        ProfilerAudioProcessor processor;
        setParameterValue(processor, "isMute", 1.0f);
        prepareProcessor(processor, 44100.0, 128);

        juce::AudioBuffer<float> buffer(2, 128);
        fillBuffer(buffer, 0.5f);
        juce::MidiBuffer midi;

        processor.processBlock(buffer, midi);

        expectClose(buffer.getMagnitude(0, buffer.getNumSamples()), 0.0f, "muted buffer magnitude");
        processor.releaseResources();
    }

    void testMasterZeroSilencesBuffer() {
        ProfilerAudioProcessor processor;
        setParameterValue(processor, "master", 0.0f);
        prepareProcessor(processor, 44100.0, 256);

        const auto rms = processSineAndMeasureRms(processor);

        expect(rms <= 0.001f, "Master volume at zero should silence the processed output.");
        processor.releaseResources();
    }

    void testGainChangesOutputLevel() {
        ProfilerAudioProcessor unityProcessor;
        prepareProcessor(unityProcessor, 44100.0, 256);
        const auto unityRms = processSineAndMeasureRms(unityProcessor);
        unityProcessor.releaseResources();

        ProfilerAudioProcessor boostedProcessor;
        setParameterValue(boostedProcessor, "gain", 6.0f);
        prepareProcessor(boostedProcessor, 44100.0, 256);
        const auto boostedRms = processSineAndMeasureRms(boostedProcessor);
        boostedProcessor.releaseResources();

        ProfilerAudioProcessor cutProcessor;
        setParameterValue(cutProcessor, "gain", -6.0f);
        prepareProcessor(cutProcessor, 44100.0, 256);
        const auto cutRms = processSineAndMeasureRms(cutProcessor);
        cutProcessor.releaseResources();

        expect(unityRms > 0.01f, "Unity-gain processor should produce measurable signal.");
        expect(boostedRms > unityRms * 1.35f, "Positive gain should increase output RMS.");
        expect(cutRms < unityRms * 0.80f, "Negative gain should decrease output RMS.");
    }

    void testSilentInputStaysSilent() {
        ProfilerAudioProcessor processor;
        prepareProcessor(processor, 48000.0, 512);

        juce::AudioBuffer<float> buffer(2, 512);
        buffer.clear();
        juce::MidiBuffer midi;

        processor.processBlock(buffer, midi);

        expect(allSamplesFinite(buffer), "Silent processed buffer should stay finite.");
        expectClose(buffer.getMagnitude(0, buffer.getNumSamples()), 0.0f, "silent input output magnitude");
        processor.releaseResources();
    }

    void testPrepareBlockSizes() {
        const int blockSizes[] = {1, 16, 128, 512};
        const double sampleRates[] = {44100.0, 48000.0};

        for (const auto sampleRate : sampleRates) {
            for (const auto blockSize : blockSizes) {
                ProfilerAudioProcessor processor;
                prepareProcessor(processor, sampleRate, blockSize);

                juce::AudioBuffer<float> buffer(2, blockSize);
                fillSineBuffer(buffer, sampleRate, 1000.0f, 0.2f);
                juce::MidiBuffer midi;

                processor.processBlock(buffer, midi);
                expect(allSamplesFinite(buffer),
                       "Processed buffer should stay finite for sample rate " + juce::String(sampleRate) +
                           " and block size " + juce::String(blockSize));
                processor.releaseResources();
            }
        }
    }

    void testReleaseAndReprepare() {
        ProfilerAudioProcessor processor;
        prepareProcessor(processor, 44100.0, 128);

        juce::AudioBuffer<float> firstBuffer(2, 128);
        fillSineBuffer(firstBuffer, 44100.0, 1000.0f, 0.2f);
        juce::MidiBuffer midi;
        processor.processBlock(firstBuffer, midi);
        processor.releaseResources();

        prepareProcessor(processor, 48000.0, 64);
        juce::AudioBuffer<float> secondBuffer(2, 64);
        fillSineBuffer(secondBuffer, 48000.0, 1000.0f, 0.2f);
        processor.processBlock(secondBuffer, midi);

        expect(allSamplesFinite(firstBuffer), "First processed buffer should stay finite.");
        expect(allSamplesFinite(secondBuffer), "Reprepared processed buffer should stay finite.");
        expect(secondBuffer.getMagnitude(0, secondBuffer.getNumSamples()) > 0.0f,
               "Reprepared processor should still produce signal.");
        processor.releaseResources();
    }

    void testMissingAssetFiles() {
        ProfilerAudioProcessor processor;
        const auto missingFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                     .getChildFile("profiler-missing-test-file.wav");

        expect(!missingFile.existsAsFile(), "Missing-file fixture should not exist.");
        expect(!processor.loadIRFile(missingFile), "Missing IR file should be rejected.");
        expect(!processor.isIRLoaded(), "Missing IR file should not mark IR as loaded.");

        expect(!processor.loadAmpFile(missingFile), "Missing amp file should be rejected.");
        expect(!processor.isAmpFileLoaded(), "Missing amp file should not mark amp as loaded.");

        processor.unloadIRFile();
        processor.unloadAmpFile();
        expect(!processor.isIRLoaded(), "Unloaded IR should stay unloaded.");
        expect(!processor.isAmpFileLoaded(), "Unloaded amp should stay unloaded.");
    }
};

AudioProcessorUnitTests audioProcessorUnitTests;

}  // namespace
}  // namespace profiler_tests
