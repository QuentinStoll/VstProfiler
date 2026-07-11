#include "TestRunner.h"

#include <JuceHeader.h>

#include <cmath>
#include <limits>
#include <utility>

#include "ProfileManager.h"

namespace profiler_tests {
namespace {

class TestAudioProcessor : public juce::AudioProcessor {
   public:
    TestAudioProcessor()
        : AudioProcessor(BusesProperties()
                             .withInput("Input", juce::AudioChannelSet::stereo(), true)
                             .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
          apvts(*this, nullptr, "Parameters", createParameterLayout()) {}

    void prepareToPlay(double, int) override {}
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override {}

    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }
    const juce::String getName() const override { return "ProfilerTestProcessor"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}
    bool isBusesLayoutSupported(const BusesLayout&) const override { return true; }

    juce::AudioProcessorValueTreeState apvts;

   private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"master", 1}, "Master Volume", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 50.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"gain", 1}, "Gain", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"noise", 1}, "Noise Gate", juce::NormalisableRange<float>(0.0f, 60.0f, 0.1f), 10.0f));
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
        layout.add(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{"isMute", 1}, "Mute", false));
        layout.add(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{"isEqEnabled", 1}, "EQ", true));

        return layout;
    }
};

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

juce::NamedValueSet makeValidProfileValues(const juce::String& profileName = "Lead") {
    juce::NamedValueSet values;
    values.set("profileName", profileName);
    values.set("masterVolume", 72.0);
    values.set("gain", -3.5);
    values.set("noiseGate", 22.0);
    values.set("bass", 4.0);
    values.set("middle", -2.0);
    values.set("treble", 5.0);
    values.set("presence", 1.5);
    values.set("depth", -1.0);
    values.set("irPath", "C:/ProfilerTests/lead-ir.wav");
    values.set("ampPath", "C:/ProfilerTests/lead-amp.json");
    return values;
}

double getNumericValue(const juce::NamedValueSet& values, const juce::Identifier& id) {
    if (const auto* value = values.getVarPointer(id)) {
        return static_cast<double>(*value);
    }

    return std::numeric_limits<double>::quiet_NaN();
}

class ProfileManagerUnitTests : public juce::UnitTest {
   public:
    ProfileManagerUnitTests()
        : juce::UnitTest("ProfileManager", "Profiler") {}

    void runTest() override {
        runCase("create profile", [this] {
            testCreateProfile();
        });

        runCase("update profile", [this] {
            testUpdateProfile();
        });

        runCase("delete profile", [this] {
            testDeleteProfile();
        });

        runCase("export profile", [this] {
            testExportProfile();
        });

        runCase("import profile", [this] {
            testImportProfile();
        });

        runCase("reject out-of-range numeric value", [this] {
            testRejectOutOfRangeNumericValue();
        });

        runCase("reject non-string path value", [this] {
            testRejectNonStringPathValue();
        });

        runCase("reject invalid profile JSON", [this] {
            testRejectInvalidProfileJson();
        });

        runCase("reject unsupported profile format", [this] {
            testRejectUnsupportedProfileFormat();
        });

        runCase("apply profile values to APVTS parameters", [this] {
            testApplyProfileMapsParameters();
        });

        runCase("save current profile setting", [this] {
            testSaveCurrentProfileSetting();
        });

        runCase("clear current profile setting", [this] {
            testClearCurrentProfileSetting();
        });

        runCase("capture current APVTS values with profile file paths", [this] {
            testGetCurrentProfileValues();
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

    ProfileManager makeProfileManager(TestAudioProcessor& processor, const juce::File& root) {
        return ProfileManager(processor.apvts,
                              root.getChildFile("Profiles"),
                              root.getChildFile("play_view_settings.json"));
    }

    void expectClose(float actual, float expected, const juce::String& label) {
        expect(std::abs(actual - expected) <= 0.001f,
               label + " expected " + juce::String(expected) + " but got " + juce::String(actual));
    }

    float getParameterValue(juce::AudioProcessorValueTreeState& apvts, const juce::String& parameterId) {
        const auto* value = apvts.getRawParameterValue(parameterId);
        expect(value != nullptr, "Missing parameter: " + parameterId);
        return value != nullptr ? value->load() : 0.0f;
    }

    void setParameterValue(juce::AudioProcessorValueTreeState& apvts,
                           const juce::String& parameterId,
                           float value) {
        auto* parameter = apvts.getParameter(parameterId);
        expect(parameter != nullptr, "Missing parameter: " + parameterId);

        if (parameter != nullptr) {
            parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
        }
    }

    void testCreateProfile() {
        auto folder = makeTestFolder();
        TestAudioProcessor processor;
        auto manager = makeProfileManager(processor, folder.folder);
        juce::String errorMessage;

        expect(manager.getProfileCount() == 0, "Expected an empty profile directory.");
        expect(manager.createProfile(makeValidProfileValues("Lead"), &errorMessage), errorMessage);
        expect(manager.getProfileCount() == 1, "Expected one created profile.");

        const auto* profile = manager.getProfile(0);
        expect(profile != nullptr, "Created profile should be available.");
        if (profile == nullptr) {
            return;
        }

        expect(profile->name == "Lead", "Profile name was not stored.");
        expect(profile->id.isNotEmpty(), "Profile id should be generated.");
        expect(profile->file.existsAsFile(), "Profile file was not written.");
        expect(profile->values.getWithDefault("profileName", {}).toString() == "Lead",
               "Profile values should include the profileName.");

        auto reloadedManager = makeProfileManager(processor, folder.folder);
        expect(reloadedManager.getProfileCount() == 1, "Created profile should reload from disk.");

        const auto* reloadedProfile = reloadedManager.getProfile(0);
        expect(reloadedProfile != nullptr, "Reloaded profile should be available.");
        if (reloadedProfile != nullptr) {
            expect(reloadedProfile->id == profile->id, "Reloaded profile should keep the generated id.");
            expect(reloadedProfile->name == "Lead", "Reloaded profile should keep the name.");
        }
    }

    void testUpdateProfile() {
        auto folder = makeTestFolder();
        TestAudioProcessor processor;
        auto manager = makeProfileManager(processor, folder.folder);
        juce::String errorMessage;

        expect(manager.createProfile(makeValidProfileValues("Lead"), &errorMessage), errorMessage);
        const auto* profile = manager.getProfile(0);
        expect(profile != nullptr, "Created profile should be available before update.");
        if (profile == nullptr) {
            return;
        }

        const auto originalId = profile->id;
        const auto originalFile = profile->file;
        auto updatedValues = makeValidProfileValues("Crunch");
        updatedValues.set("gain", -6.0);

        errorMessage.clear();
        expect(manager.updateProfile(0, updatedValues, &errorMessage), errorMessage);
        expect(manager.getProfileCount() == 1, "Updating should not create extra profiles.");

        const auto* updatedProfile = manager.getProfile(0);
        expect(updatedProfile != nullptr, "Updated profile should still be available.");
        if (updatedProfile != nullptr) {
            expect(updatedProfile->id == originalId, "Update should keep the profile id.");
            expect(updatedProfile->file == originalFile, "Update should keep the same profile file.");
            expect(updatedProfile->name == "Crunch", "Update should store the new profile name.");
            expectClose(static_cast<float>(getNumericValue(updatedProfile->values, "gain")), -6.0f, "Updated gain");
        }
    }

    void testDeleteProfile() {
        auto folder = makeTestFolder();
        TestAudioProcessor processor;
        auto manager = makeProfileManager(processor, folder.folder);
        juce::String errorMessage;

        expect(manager.createProfile(makeValidProfileValues("Lead"), &errorMessage), errorMessage);
        const auto* profile = manager.getProfile(0);
        expect(profile != nullptr, "Created profile should be available before delete.");
        if (profile == nullptr) {
            return;
        }

        const auto originalFile = profile->file;
        errorMessage.clear();
        expect(manager.deleteProfile(0, &errorMessage), errorMessage);
        expect(manager.getProfileCount() == 0, "Deleting should remove the profile from memory.");
        expect(!originalFile.existsAsFile(), "Deleting should remove the profile file.");
    }

    void testExportProfile() {
        auto folder = makeTestFolder();
        TestAudioProcessor processor;
        auto manager = makeProfileManager(processor, folder.folder);
        juce::String errorMessage;

        const auto exportDirectory = folder.folder.getChildFile("Exports");
        const auto createDirectoryResult = exportDirectory.createDirectory();
        expect(createDirectoryResult.wasOk(), createDirectoryResult.getErrorMessage());

        const auto destinationWithoutExtension = exportDirectory.getChildFile("Shared Lead");
        expect(manager.exportProfile(makeValidProfileValues("Shared Lead"),
                                     destinationWithoutExtension,
                                     &errorMessage),
               errorMessage);

        const auto exportedFile = destinationWithoutExtension.withFileExtension(ProfileManager::profileFileExtension);
        expect(exportedFile.existsAsFile(), "Export should add the profile extension.");

        const auto parsedExport = juce::JSON::parse(exportedFile.loadFileAsString());
        const auto* exportedObject = parsedExport.getDynamicObject();
        expect(exportedObject != nullptr, "Exported profile should be valid JSON.");
        if (exportedObject != nullptr) {
            expect(exportedObject->getProperty("format").toString() == "profilerprofile",
                   "Exported profile should use the profilerprofile format.");
            expect(exportedObject->getProperty("name").toString() == "Shared Lead",
                   "Exported profile should store the profile name.");
        }
    }

    void testImportProfile() {
        auto folder = makeTestFolder();
        TestAudioProcessor processor;
        auto manager = makeProfileManager(processor, folder.folder);
        juce::String errorMessage;

        const auto exportDirectory = folder.folder.getChildFile("Exports");
        const auto createDirectoryResult = exportDirectory.createDirectory();
        expect(createDirectoryResult.wasOk(), createDirectoryResult.getErrorMessage());

        const auto exportedFile = exportDirectory.getChildFile("Shared Lead" + juce::String(ProfileManager::profileFileExtension));
        expect(manager.exportProfile(makeValidProfileValues("Shared Lead"),
                                     exportedFile,
                                     &errorMessage),
               errorMessage);

        const auto parsedExport = juce::JSON::parse(exportedFile.loadFileAsString());
        const auto* exportedObject = parsedExport.getDynamicObject();
        expect(exportedObject != nullptr, "Exported profile should be valid JSON before import.");
        const auto exportedId = exportedObject != nullptr
                                    ? exportedObject->getProperty("id").toString()
                                    : juce::String{};

        errorMessage.clear();
        expect(manager.importProfile(exportedFile, &errorMessage), errorMessage);
        expect(manager.getProfileCount() == 1, "Import should create one local profile.");

        const auto* importedProfile = manager.getProfile(0);
        expect(importedProfile != nullptr, "Imported profile should be available.");
        if (importedProfile != nullptr) {
            expect(importedProfile->name == "Shared Lead", "Import should preserve profile name.");
            expect(importedProfile->id.isNotEmpty(), "Import should assign an id.");
            expect(importedProfile->id != exportedId, "Import should assign a fresh local id.");
        }
    }

    void testRejectOutOfRangeNumericValue() {
        auto folder = makeTestFolder();
        TestAudioProcessor processor;
        auto manager = makeProfileManager(processor, folder.folder);
        juce::String errorMessage;

        auto invalidGain = makeValidProfileValues("Invalid Gain");
        invalidGain.set("gain", 99.0);
        expect(!manager.createProfile(invalidGain, &errorMessage), "Out-of-range gain should be rejected.");
        expect(errorMessage.contains("gain"), "Gain validation error should mention gain.");
    }

    void testRejectNonStringPathValue() {
        auto folder = makeTestFolder();
        TestAudioProcessor processor;
        auto manager = makeProfileManager(processor, folder.folder);
        juce::String errorMessage;

        auto invalidPath = makeValidProfileValues("Invalid Path");
        invalidPath.set("irPath", 123.0);
        expect(!manager.createProfile(invalidPath, &errorMessage), "Non-string path should be rejected.");
        expect(errorMessage.contains("irPath"), "Path validation error should mention irPath.");
    }

    void testRejectInvalidProfileJson() {
        auto folder = makeTestFolder();
        TestAudioProcessor processor;
        auto manager = makeProfileManager(processor, folder.folder);
        juce::String errorMessage;

        const auto badJsonFile = folder.folder.getChildFile("bad.profilerprofile");
        expect(badJsonFile.replaceWithText("{"), "Could not write bad JSON fixture.");
        expect(!manager.importProfile(badJsonFile, &errorMessage), "Bad JSON should be rejected.");
        expect(errorMessage.contains("Invalid profile JSON"), "Bad JSON error should be explicit.");
    }

    void testRejectUnsupportedProfileFormat() {
        auto folder = makeTestFolder();
        TestAudioProcessor processor;
        auto manager = makeProfileManager(processor, folder.folder);
        juce::String errorMessage;

        const auto unsupportedFile = folder.folder.getChildFile("unsupported.profilerprofile");
        expect(unsupportedFile.replaceWithText(R"({"format":"other","parameters":{}})"),
               "Could not write unsupported-format fixture.");
        expect(!manager.importProfile(unsupportedFile, &errorMessage), "Unsupported format should be rejected.");
        expect(errorMessage.contains("Unsupported profile format"), "Unsupported-format error should be explicit.");
    }

    void testApplyProfileMapsParameters() {
        auto folder = makeTestFolder();
        TestAudioProcessor processor;
        auto manager = makeProfileManager(processor, folder.folder);
        juce::String errorMessage;

        auto values = makeValidProfileValues("Mapped");
        values.set("masterVolume", 81.0);
        values.set("gain", 6.5);
        values.set("noiseGate", 31.0);
        values.set("bass", -7.0);
        values.set("middle", 8.0);
        values.set("treble", -9.0);
        values.set("presence", 10.0);
        values.set("depth", -11.0);

        expect(manager.createProfile(values, &errorMessage), errorMessage);
        errorMessage.clear();
        expect(manager.applyProfile(0, &errorMessage), errorMessage);

        expectClose(getParameterValue(processor.apvts, "master"), 81.0f, "masterVolume -> master");
        expectClose(getParameterValue(processor.apvts, "gain"), 6.5f, "gain -> gain");
        expectClose(getParameterValue(processor.apvts, "noise"), 31.0f, "noiseGate -> noise");
        expectClose(getParameterValue(processor.apvts, "bass"), -7.0f, "bass -> bass");
        expectClose(getParameterValue(processor.apvts, "mid"), 8.0f, "middle -> mid");
        expectClose(getParameterValue(processor.apvts, "treble"), -9.0f, "treble -> treble");
        expectClose(getParameterValue(processor.apvts, "presence"), 10.0f, "presence -> presence");
        expectClose(getParameterValue(processor.apvts, "depth"), -11.0f, "depth -> depth");
    }

    void testSaveCurrentProfileSetting() {
        auto folder = makeTestFolder();
        const auto settingsFile = folder.folder.getChildFile("play_view_settings.json");
        const auto profileDirectory = folder.folder.getChildFile("Profiles");
        juce::String savedProfileId;

        TestAudioProcessor processor;
        ProfileManager manager(processor.apvts, profileDirectory, settingsFile);
        juce::String errorMessage;
        expect(manager.createProfile(makeValidProfileValues("Persistent"), &errorMessage), errorMessage);

        const auto* profile = manager.getProfile(0);
        expect(profile != nullptr, "Created profile should be available for persistence.");
        if (profile == nullptr) {
            return;
        }

        savedProfileId = profile->id;
        errorMessage.clear();
        expect(manager.applyProfileAndSaveAsCurrent(0, &errorMessage), errorMessage);
        expect(settingsFile.existsAsFile(), "Current-profile settings file should be written.");

        TestAudioProcessor reloadedProcessor;
        ProfileManager reloadedManager(reloadedProcessor.apvts, profileDirectory, settingsFile);
        expect(reloadedManager.getCurrentProfileId() == savedProfileId,
               "Reloaded manager should restore the current profile id.");
        expect(reloadedManager.getCurrentProfileIndex() == 0,
               "Reloaded manager should resolve the current profile id.");
    }

    void testClearCurrentProfileSetting() {
        auto folder = makeTestFolder();
        const auto settingsFile = folder.folder.getChildFile("play_view_settings.json");
        const auto profileDirectory = folder.folder.getChildFile("Profiles");

        {
            TestAudioProcessor processor;
            ProfileManager manager(processor.apvts, profileDirectory, settingsFile);
            juce::String errorMessage;
            expect(manager.createProfile(makeValidProfileValues("Persistent"), &errorMessage), errorMessage);
            expect(manager.applyProfileAndSaveAsCurrent(0, &errorMessage), errorMessage);
            manager.clearCurrentProfile();
        }

        {
            TestAudioProcessor processor;
            ProfileManager manager(processor.apvts, profileDirectory, settingsFile);
            expect(manager.getCurrentProfileId().isEmpty(),
                   "Cleared current profile should stay cleared after reload.");
            expect(manager.getCurrentProfileIndex() == -1,
                   "Cleared current profile should not resolve to a profile index.");
        }
    }

    void testGetCurrentProfileValues() {
        auto folder = makeTestFolder();
        TestAudioProcessor processor;
        auto manager = makeProfileManager(processor, folder.folder);
        juce::String errorMessage;

        const auto values = makeValidProfileValues("Source Profile");
        expect(manager.createProfile(values, &errorMessage), errorMessage);

        const auto* profile = manager.getProfile(0);
        expect(profile != nullptr, "Profile should be available.");
        if (profile == nullptr) {
            return;
        }

        manager.setCurrentProfileId(profile->id);

        setParameterValue(processor.apvts, "master", 42.0f);
        setParameterValue(processor.apvts, "gain", 3.0f);
        setParameterValue(processor.apvts, "noise", 14.0f);
        setParameterValue(processor.apvts, "mid", -5.0f);

        const auto capturedValues = manager.getCurrentProfileValues("Captured Profile");

        expect(capturedValues.getWithDefault("profileName", {}).toString() == "Captured Profile",
               "Captured values should use the requested profile name.");
        expect(capturedValues.getWithDefault("irPath", {}).toString() == "C:/ProfilerTests/lead-ir.wav",
               "Captured values should preserve the current profile IR path.");
        expect(capturedValues.getWithDefault("ampPath", {}).toString() == "C:/ProfilerTests/lead-amp.json",
               "Captured values should preserve the current profile amp path.");
        expectClose(static_cast<float>(getNumericValue(capturedValues, "masterVolume")), 42.0f, "Captured master");
        expectClose(static_cast<float>(getNumericValue(capturedValues, "gain")), 3.0f, "Captured gain");
        expectClose(static_cast<float>(getNumericValue(capturedValues, "noiseGate")), 14.0f, "Captured noise gate");
        expectClose(static_cast<float>(getNumericValue(capturedValues, "middle")), -5.0f, "Captured middle");
    }
};

ProfileManagerUnitTests profileManagerUnitTests;

}  // namespace
}  // namespace profiler_tests
