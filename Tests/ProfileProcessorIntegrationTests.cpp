#include <JuceHeader.h>

#include <cmath>
#include <utility>

#include "PluginProcessor.h"
#include "TestRunner.h"

namespace profiler_tests {
namespace {

struct ScopedTestFolder {
    explicit ScopedTestFolder(juce::File folderIn)
        : folder(std::move(folderIn)) {}

    ScopedTestFolder(ScopedTestFolder&& other) noexcept
        : folder(other.folder) {
        other.folder = juce::File{};
    }

    ~ScopedTestFolder() {
        if (folder != juce::File{}) {
            folder.deleteRecursively();
        }
    }

    ScopedTestFolder(const ScopedTestFolder&) = delete;
    ScopedTestFolder& operator=(const ScopedTestFolder&) = delete;
    ScopedTestFolder& operator=(ScopedTestFolder&&) = delete;

    juce::File folder;
};

class ProfileProcessorIntegrationTests : public juce::UnitTest {
   public:
    ProfileProcessorIntegrationTests()
        : juce::UnitTest("ProfileProcessorIntegration", "Profiler") {}

    void runTest() override {
        runCase("profile processor apply maps profile values", [this] {
            testApplyProfileMapsValuesThroughProcessor();
        });

        runCase("profile processor tracks profile asset paths", [this] {
            testApplyProfileTracksAssetPaths();
        });

        runCase("profile processor applied profile affects audio", [this] {
            testAppliedProfileAffectsAudioProcessing();
        });

        runCase("profile processor syncs current profile selection", [this] {
            testSyncCurrentProfileSelection();
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

    ScopedTestFolder makeTestFolder() {
        const auto baseFolder = juce::File(PROFILER_TEST_TEMP_ROOT);
        const auto baseResult = baseFolder.createDirectory();
        expect(baseResult.wasOk(), "Could not create test temp root: " + baseResult.getErrorMessage());

        const auto folder = baseFolder.getChildFile(juce::Uuid().toDashedString());
        const auto result = folder.createDirectory();
        expect(result.wasOk(), "Could not create test folder: " + result.getErrorMessage());
        return ScopedTestFolder(folder);
    }

    ProfilerAudioProcessor makeProcessor(const juce::File& root) {
        return ProfilerAudioProcessor(root.getChildFile("Profiles"),
                                      root.getChildFile("play_view_settings.json"));
    }

    juce::NamedValueSet makeProfileValues(const juce::String& profileName,
                                          double masterVolume = 72.0,
                                          double gain = 3.0,
                                          const juce::String& irPath = {},
                                          const juce::String& ampPath = {}) {
        juce::NamedValueSet values;
        values.set("profileName", profileName);
        values.set("masterVolume", masterVolume);
        values.set("gain", gain);
        values.set("noiseGate", 18.0);
        values.set("bass", 2.0);
        values.set("middle", -3.0);
        values.set("treble", 4.0);
        values.set("presence", 5.0);
        values.set("depth", -6.0);
        values.set("irPath", irPath);
        values.set("ampPath", ampPath);
        return values;
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

    void prepareProcessor(ProfilerAudioProcessor& processor, double sampleRate, int blockSize) {
        processor.setRateAndBufferSizeDetails(sampleRate, blockSize);
        processor.prepareToPlay(sampleRate, blockSize);
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

    float processSineAndMeasureRms(ProfilerAudioProcessor& processor) {
        constexpr double sampleRate = 44100.0;
        constexpr int blockSize = 256;
        juce::AudioBuffer<float> buffer(2, blockSize);
        juce::MidiBuffer midi;

        for (int block = 0; block < 32; ++block) {
            fillSineBuffer(buffer, sampleRate, 1000.0f, 0.25f);
            processor.processBlock(buffer, midi);
        }

        expect(allSamplesFinite(buffer), "Integrated processor output should stay finite.");
        return juce::jmax(buffer.getRMSLevel(0, 0, buffer.getNumSamples()),
                          buffer.getRMSLevel(1, 0, buffer.getNumSamples()));
    }

    void testApplyProfileMapsValuesThroughProcessor() {
        auto folder = makeTestFolder();
        auto processor = makeProcessor(folder.folder);
        juce::String errorMessage;

        expect(processor.getProfileManager().createProfile(makeProfileValues("Integrated Lead"), &errorMessage),
               errorMessage);

        const auto* profile = processor.getProfileManager().getProfile(0);
        expect(profile != nullptr, "Created integration profile should be available.");
        if (profile == nullptr) {
            return;
        }

        errorMessage.clear();
        expect(processor.applyProfile(0, &errorMessage), errorMessage);

        expectClose(getParameterValue(processor, "master"), 72.0f, "profile master -> processor master");
        expectClose(getParameterValue(processor, "gain"), 3.0f, "profile gain -> processor gain");
        expectClose(getParameterValue(processor, "noise"), 18.0f, "profile noiseGate -> processor noise");
        expectClose(getParameterValue(processor, "bass"), 2.0f, "profile bass -> processor bass");
        expectClose(getParameterValue(processor, "mid"), -3.0f, "profile middle -> processor mid");
        expectClose(getParameterValue(processor, "treble"), 4.0f, "profile treble -> processor treble");
        expectClose(getParameterValue(processor, "presence"), 5.0f, "profile presence -> processor presence");
        expectClose(getParameterValue(processor, "depth"), -6.0f, "profile depth -> processor depth");
        expect(processor.getAppliedProfileId() == profile->id,
               "Processor should remember the applied profile id.");
    }

    void testApplyProfileTracksAssetPaths() {
        auto folder = makeTestFolder();
        auto processor = makeProcessor(folder.folder);
        juce::String errorMessage;

        const auto missingIR = folder.folder.getChildFile("missing-ir.wav");
        const auto missingAmp = folder.folder.getChildFile("missing-amp.json");
        expect(!missingIR.existsAsFile(), "Missing IR fixture should not exist.");
        expect(!missingAmp.existsAsFile(), "Missing amp fixture should not exist.");

        expect(processor.getProfileManager().createProfile(makeProfileValues("Assets",
                                                                             70.0,
                                                                             0.0,
                                                                             missingIR.getFullPathName(),
                                                                             missingAmp.getFullPathName()),
                                                           &errorMessage),
               errorMessage);

        errorMessage.clear();
        expect(processor.applyProfile(0, &errorMessage), errorMessage);

        expect(!processor.isIRLoaded(), "Missing profile IR should not be marked as loaded.");
        expect(!processor.isAmpFileLoaded(), "Missing profile amp should not be marked as loaded.");
        expect(processor.getCurrentIRFile() == missingIR,
               "Processor should still track the profile IR path for the UI.");
        expect(processor.getCurrentAmpFile() == missingAmp,
               "Processor should still track the profile amp path for the UI.");
    }

    void testAppliedProfileAffectsAudioProcessing() {
        auto folder = makeTestFolder();
        auto processor = makeProcessor(folder.folder);
        juce::String errorMessage;

        expect(processor.getProfileManager().createProfile(makeProfileValues("Silent Profile", 0.0, 0.0),
                                                           &errorMessage),
               errorMessage);

        errorMessage.clear();
        expect(processor.applyProfile(0, &errorMessage), errorMessage);

        prepareProcessor(processor, 44100.0, 256);
        const auto rms = processSineAndMeasureRms(processor);

        expect(rms <= 0.001f,
               "A profile with masterVolume 0 should silence audio through the processor.");
        processor.releaseResources();
    }

    void testSyncCurrentProfileSelection() {
        auto folder = makeTestFolder();
        auto processor = makeProcessor(folder.folder);
        juce::String errorMessage;

        const auto missingIR = folder.folder.getChildFile("current-profile-ir.wav");
        expect(processor.getProfileManager().createProfile(makeProfileValues("Current",
                                                                             65.0,
                                                                             1.0,
                                                                             missingIR.getFullPathName(),
                                                                             {}),
                                                           &errorMessage),
               errorMessage);

        const auto* profile = processor.getProfileManager().getProfile(0);
        expect(profile != nullptr, "Created current profile should be available.");
        if (profile == nullptr) {
            return;
        }

        processor.getProfileManager().setCurrentProfileId(profile->id);
        processor.syncLoadedFilesWithCurrentProfile();

        expect(processor.getAppliedProfileId() == profile->id,
               "Processor should remember the synced current profile id.");
        expect(processor.getCurrentIRFile() == missingIR,
               "Sync should apply current profile file paths to the processor.");
        expect(!processor.isIRLoaded(), "Missing synced IR should not be marked as loaded.");
    }
};

ProfileProcessorIntegrationTests profileProcessorIntegrationTests;

}  // namespace
}  // namespace profiler_tests
