#include <JuceHeader.h>

#include "Logging.h"
#include "TestRunner.h"

class LoggingUnitTests : public juce::UnitTest {
   public:
    LoggingUnitTests() : juce::UnitTest("Logging", "Profiler") {}

    void runTest() override {
        runCase("create and find logger", [this] { testCreateAndFind(); });
        runCase("duplicate create throws", [this] { testDuplicateCreateThrows(); });
        runCase("get non-existent throws", [this] { testGetNonExistentThrows(); });
        runCase("format log labels", [this] { testLogLabels(); });
        runCase("parse config and write log file", [this] { testLogConfigFromFileAndFileLogging(); });
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

    void testLogLabels() {
        expectEquals(juce::String(Log::toString(Log::LogLevel::Trace)), juce::String("[Trace]"));
        expectEquals(juce::String(Log::toString(Log::LogLevel::Debug)), juce::String("[Debug]"));
        expectEquals(juce::String(Log::toString(Log::LogLevel::Info)), juce::String("[Info]"));
        expectEquals(juce::String(Log::toString(Log::LogLevel::Warning)), juce::String("[Warning]"));
        expectEquals(juce::String(Log::toString(Log::LogLevel::Error)), juce::String("[Error]"));
        expectEquals(juce::String(Log::toString(Log::LogLevel::Fatal)), juce::String("[Fatal]"));
        expectEquals(juce::String(Log::toString(Log::LogLevel::Other)), juce::String("[Other]"));

        expectEquals(juce::String(Log::toString(Log::LogCategory::Init)), juce::String("[Init]"));
        expectEquals(juce::String(Log::toString(Log::LogCategory::Dsp)), juce::String("[Dsp]"));
        expectEquals(juce::String(Log::toString(Log::LogCategory::Io)), juce::String("[Io]"));
        expectEquals(juce::String(Log::toString(Log::LogCategory::Ui)), juce::String("[Ui]"));
        expectEquals(juce::String(Log::toString(Log::LogCategory::Param)), juce::String("[Param]"));
        expectEquals(juce::String(Log::toString(Log::LogCategory::Host)), juce::String("[Host]"));
        expectEquals(juce::String(Log::toString(Log::LogCategory::Perf)), juce::String("[Perf]"));
        expectEquals(juce::String(Log::toString(Log::LogCategory::Other)), juce::String("[Other]"));
    }

    void testLogConfigFromFileAndFileLogging() {
        Log::LogRegistry::shutdownAll();

        // Prepare a temp folder for logs
        const auto baseFolder = juce::File(PROFILER_TEST_TEMP_ROOT).getChildFile(juce::Uuid().toDashedString());
        const auto folderResult = baseFolder.createDirectory();
        expect(folderResult.wasOk(), "Could not create temp folder");

        juce::var configObject(new juce::DynamicObject());
        configObject.getDynamicObject()->setProperty("name", "cfgtest");
        configObject.getDynamicObject()->setProperty("log_level", "Debug");
        configObject.getDynamicObject()->setProperty("show_in_ui", false);
        configObject.getDynamicObject()->setProperty("write_to_file", true);
        configObject.getDynamicObject()->setProperty("write_to_debug", false);
        configObject.getDynamicObject()->setProperty("write_to_tracy", false);
        configObject.getDynamicObject()->setProperty("log_directory", baseFolder.getFullPathName());

        const auto cfgFile = baseFolder.getChildFile("log_settings.json");
        expect(cfgFile.replaceWithText(juce::JSON::toString(configObject, true)),
               "Could not write config JSON");

        auto cfg = Log::LogConfig::fromFile(cfgFile);
        expect(cfg.name == "cfgtest", "Config name should be parsed");
        expect(cfg.logLevel == Log::LogLevel::Debug, "Config log level should be Debug");
        expect(cfg.writeToFile == true, "Config writeToFile should be true");
        expect(cfg.logDirectory.exists(), "Config logDirectory should exist");
        expect(cfg.logFile.existsAsFile(), "Derived log file should exist when writeToFile and directory exist");

        // Create a logger that writes to the derived file and test writing/filtering
        cfg.writeToDebug = false;
        cfg.writeToTracy = false;
        Log::logSystemInfoOnFileStart = true;
        auto& logger = Log::LogRegistry::create("file_logger", cfg);

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
        Log::logSystemInfoOnFileStart = false;
        baseFolder.deleteRecursively();
    }
};

static LoggingUnitTests loggingUnitTests;
