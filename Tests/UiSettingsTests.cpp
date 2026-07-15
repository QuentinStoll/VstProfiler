#include "SettingsPath.h"
#include "TestRunner.h"
#include "UiSettings.h"

namespace profiler_tests {
namespace {

class ScopedUiSettingsFile {
   public:
    ScopedUiSettingsFile()
        : file(getFileInSettingsFolder("ui_settings.json")),
          existed(file.existsAsFile()),
          originalContents(existed ? file.loadFileAsString() : juce::String{}) {}

    ~ScopedUiSettingsFile() {
        if (existed) {
            file.replaceWithText(originalContents);
        } else {
            file.deleteFile();
        }
    }

    juce::File file;

   private:
    bool existed;
    juce::String originalContents;
};

class UiSettingsUnitTests : public juce::UnitTest {
   public:
    UiSettingsUnitTests()
        : juce::UnitTest("UiSettings", "Profiler") {}

    void runTest() override {
        runCase("ui settings defaults and normalisation", [this] {
            testDefaultsAndNormalisation();
        });
        runCase("ui settings persistence", [this] {
            testPersistence();
        });
        runCase("ui settings invalid JSON", [this] {
            testInvalidJson();
        });
    }

   private:
    template <typename Fn>
    void runCase(const juce::String& name, Fn&& fn) {
        if (requestedTestCase.isNotEmpty() && requestedTestCase != name) {
            return;
        }

        beginTest(name);
        fn();
    }

    void testDefaultsAndNormalisation() {
        ScopedUiSettingsFile settings;
        settings.file.deleteFile();

        expectEquals(UiSettings::loadBackgroundColourPreset(), 1);
        expectEquals(UiSettings::loadTroubleshootingSetting(), 1);
        expectEquals(UiSettings::loadHardwareInfoSetting(), 1);
        expectEquals(UiSettings::normaliseBackgroundColourPreset(3), 3);
        expectEquals(UiSettings::normaliseBackgroundColourPreset(0), 1);
        expectEquals(UiSettings::normaliseBackgroundColourPreset(6), 1);
    }

    void testPersistence() {
        ScopedUiSettingsFile settings;
        settings.file.deleteFile();

        UiSettings::saveBackgroundColourPreset(4);
        UiSettings::saveTroubleshootingSetting(2);
        UiSettings::saveHardwareInfoSetting(2);

        expectEquals(UiSettings::loadBackgroundColourPreset(), 4);
        expectEquals(UiSettings::loadTroubleshootingSetting(), 2);
        expectEquals(UiSettings::loadHardwareInfoSetting(), 2);

        UiSettings::saveBackgroundColourPreset(99);
        UiSettings::saveTroubleshootingSetting(99);
        UiSettings::saveHardwareInfoSetting(99);

        expectEquals(UiSettings::loadBackgroundColourPreset(), 1);
        expectEquals(UiSettings::loadTroubleshootingSetting(), 1);
        expectEquals(UiSettings::loadHardwareInfoSetting(), 1);
    }

    void testInvalidJson() {
        ScopedUiSettingsFile settings;
        expect(settings.file.replaceWithText("not valid JSON"));

        expectEquals(UiSettings::loadBackgroundColourPreset(), 1);
        expectEquals(UiSettings::loadTroubleshootingSetting(), 1);
        expectEquals(UiSettings::loadHardwareInfoSetting(), 1);
    }
};

UiSettingsUnitTests uiSettingsUnitTests;

}  // namespace
}  // namespace profiler_tests
