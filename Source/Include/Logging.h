#pragma once
#include <JuceHeader.h>


enum class LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL,
    OTHER
};

enum class LogCategory {
    INIT,
    DSP,
    IO,
    UI,
    PARAM,
    HOST,
    PERF,
    OTHER
};


struct LoggingConfig {
    LogLevel logLevel = LogLevel::INFO;
    bool showInUI = false;
    bool writeToFile = false;
    bool writeToDebug = true;
    juce::File logDirectory = juce::File();
    juce::File logFile = juce::File();
};


const char* logLevelToString(LogLevel level) noexcept;
const char* logCategoryToString(LogCategory category) noexcept;


class LoggingConfigLoader final {
    public:
        static LoggingConfig load();
        static LoggingConfig loadFromFile(const juce::File& file);
        static LoggingConfig loadDefaultConfigFile();

    private:
        LoggingConfigLoader() = delete;
};


class AppLogger final {
    public:
        static void initialise();
        static void initialise(LoggingConfig config);
        static void shutdown();

        static void log(LogLevel level, LogCategory category, const juce::String& message);

        static void trace (LogCategory category, const juce::String& message);
        static void debug (LogCategory category, const juce::String& message);
        static void info  (LogCategory category, const juce::String& message);
        static void warn  (LogCategory category, const juce::String& message);
        static void error (LogCategory category, const juce::String& message);
        static void fatal (LogCategory category, const juce::String& message);

        static bool isInitialised() noexcept;
        static const LoggingConfig& getConfig();
        static void reloadConfig();


    private:
        AppLogger() = delete;
        ~AppLogger() = delete;
        AppLogger(const AppLogger&) = delete;
        AppLogger& operator=(const AppLogger&) = delete;

        static bool initialised;
        static LoggingConfig currentConfig;
};
