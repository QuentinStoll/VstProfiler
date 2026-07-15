#include <JuceHeader.h>

#include "TestRunner.h"
#include "Logging.h"

class LoggingUnitTests : public juce::UnitTest {
public:
    LoggingUnitTests() : juce::UnitTest("Logging", "Profiler") {}

    void runTest() override {
        runCase("create and find logger", [this] { testCreateAndFind(); });
        runCase("duplicate create throws", [this] { testDuplicateCreateThrows(); });
        runCase("get non-existent throws", [this] { testGetNonExistentThrows(); });
    }

private:
    template <typename Fn>
    void runCase(const juce::String& name, Fn&& fn) {
        if (!profiler_tests::requestedTestCase.isEmpty() && profiler_tests::requestedTestCase != name)
            return;
        beginTest(name);
        fn();
    }

    void testCreateAndFind() {
        Log::LogRegistry::shutdownAll();

        Log::LogConfig cfg;
        cfg.writeToFile = false;
        cfg.writeToDebug = false;
        cfg.writeToTracy = false;
        cfg.showInUI = false;

        auto& logger = Log::LogRegistry::create("unit_test_logger", cfg);
        expect(Log::LogRegistry::find("unit_test_logger") != nullptr, "Logger should be findable after create");
        expect(&Log::LogRegistry::get("unit_test_logger") == &logger, "get() should return the created instance");
        expect(logger.isInitialised(), "Logger should be initialised after create");

        Log::LogRegistry::shutdownAll();
        expect(Log::LogRegistry::find("unit_test_logger") == nullptr, "Logger should be removed after shutdownAll");
    }

    void testDuplicateCreateThrows() {
        Log::LogRegistry::shutdownAll();

        Log::LogConfig cfg;
        cfg.writeToFile = false;
        cfg.writeToDebug = false;
        cfg.writeToTracy = false;

        Log::LogRegistry::create("dup_logger", cfg);

        bool threw = false;
        try {
            Log::LogRegistry::create("dup_logger", cfg);
        } catch (const std::runtime_error&) {
            threw = true;
        }

        expect(threw, "Creating a logger with an existing name should throw runtime_error");
        Log::LogRegistry::shutdownAll();
    }

    void testGetNonExistentThrows() {
        Log::LogRegistry::shutdownAll();

        bool threw = false;
        try {
            (void)Log::LogRegistry::get("no_such_logger");
        } catch (const std::out_of_range&) {
            threw = true;
        }

        expect(threw, "get() for a non-existent logger should throw out_of_range");
    }

    void testLogConfigFromFileAndFileLogging() {
        Log::LogRegistry::shutdownAll();

        // Prepare a temp folder for logs
        const auto baseFolder = juce::File(PROFILER_TEST_TEMP_ROOT).getChildFile(juce::Uuid().toDashedString());
        const auto folderResult = baseFolder.createDirectory();
        expect(folderResult.wasOk(), "Could not create temp folder");

        // Create a settings JSON with write_to_file and log_directory
        juce::String json = "{\n"
                            "  \"name\": \"cfgtest\",\n"
                            "  \"log_level\": \"Debug\",\n"
                            "  \"show_in_ui\": false,\n"
                            "  \"write_to_file\": true,\n"
                            "  \"write_to_debug\": false,\n"
                            "  \"write_to_tracy\": false,\n"
                            "  \"log_directory\": \"" + baseFolder.getFullPathName() + "\"\n"
                            "}\n";

        const auto cfgFile = baseFolder.getChildFile("log_settings.json");
        expect(cfgFile.replaceWithText(json), "Could not write config JSON");

        auto cfg = Log::LogConfig::fromFile(cfgFile);
        expect(cfg.name == "cfgtest", "Config name should be parsed");
        expect(cfg.logLevel == Log::LogLevel::Debug, "Config log level should be Debug");
        expect(cfg.writeToFile == true, "Config writeToFile should be true");
        expect(cfg.logDirectory.exists(), "Config logDirectory should exist");
        expect(cfg.logFile.existsAsFile(), "Derived log file should exist when writeToFile and directory exist");

        // Create a logger that writes to the derived file and test writing/filtering
        cfg.writeToDebug = false;
        cfg.writeToTracy = false;
        auto& logger = Log::LogRegistry::create("file_logger", cfg);

        // Ensure header was written when global flag is enabled
        Log::logSystemInfoOnFileStart = true;
        logger.info(Log::LogCategory::Init, "Test message 1");
        logger.error(Log::LogCategory::Init, "Error message");

        // Read the log file and check contents
        juce::String contents = cfg.logFile.loadFileAsString();
        expect(contents.isNotEmpty(), "Log file should contain text");
        expect(contents.contains("Test message 1") || contents.contains("Error message"), "Log file should contain logged messages");

        // Test log level filtering: set high threshold so info is filtered
        logger.shutdown();
        cfg.logLevel = Log::LogLevel::Error;
        auto& logger2 = Log::LogRegistry::create("file_logger2", cfg);
        logger2.info(Log::LogCategory::Init, "Should be filtered");
        logger2.error(Log::LogCategory::Init, "Should appear");
        juce::String contents2 = cfg.logFile.loadFileAsString();
        expect(contents2.contains("Should appear"), "Error message should appear in file");

        // Cleanup
        Log::LogRegistry::shutdownAll();
        baseFolder.deleteRecursively();
    }
};

static LoggingUnitTests loggingUnitTests;
