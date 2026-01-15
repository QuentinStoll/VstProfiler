#include <simdjson.h>
#include <string_view>
#include "Logging.h"
#include "SettingsPath.h"
#include "juce_core/juce_core.h"


const char* logLevelToString(LogLevel level) noexcept {
    switch (level) {
        case LogLevel::TRACE:
            return "[TRACE]";
        case LogLevel::DEBUG:
            return "[DEBUG]";
        case LogLevel::INFO:
            return "[INFO]";
        case LogLevel::WARNING:
            return "[WARNING]";
        case LogLevel::ERROR:
            return "[ERROR]";
        case LogLevel::FATAL:
            return "[FATAL]";
        default:
            return "[OTHER]";
    }
}

const char* logCategoryToString(LogCategory category) noexcept {
    switch (category) {
        case LogCategory::INIT:
            return "[INIT]";
        case LogCategory::DSP:
            return "[DSP]";
        case LogCategory::IO:
            return "[IO]";
        case LogCategory::UI:
            return "[UI]";
        case LogCategory::PARAM:
            return "[PARAM]";
        case LogCategory::HOST:
            return "[HOST]";
        case LogCategory::PERF:
            return "[PERF]";
        default:
            return "[OTHER]";
    }
}


//LoggingConfigLoader
// Load config from default platform path
LoggingConfig LoggingConfigLoader::load() {
    return loadFromFile(getFileInSettingsFolder("log_settings.json"));
}

// Load config from a specific file
LoggingConfig LoggingConfigLoader::loadFromFile(const juce::File& file) {
    LoggingConfig config; // defaults already set in struct
    if (!file.existsAsFile()) return config;
    juce::String jsonText = file.loadFileAsString();
    if (jsonText.isEmpty()) return config;
    simdjson::dom::parser parser;
    simdjson::dom::element root;
    auto error = parser.parse(jsonText.toRawUTF8(), jsonText.getNumBytesAsUTF8()).get(root);
    if (error) return config;

    // log_level
    std::string_view _logLevel;
    if (root["log_level"].get(_logLevel) == simdjson::SUCCESS) {
        if      (_logLevel == "TRACE")      { config.logLevel = LogLevel::TRACE; }
        else if (_logLevel == "DEBUG")      { config.logLevel = LogLevel::DEBUG; }
        else if (_logLevel == "INFO")       { config.logLevel = LogLevel::INFO; }
        else if (_logLevel == "WARNING")    { config.logLevel = LogLevel::WARNING; }
        else if (_logLevel == "ERROR")      { config.logLevel = LogLevel::ERROR; }
        else if (_logLevel == "FATAL")      { config.logLevel = LogLevel::FATAL; }
        else                                { config.logLevel = LogLevel::OTHER; }
    }

    // show_in_ui
    bool _showInUI;
    if (root["show_in_ui"].get(_showInUI) == simdjson::SUCCESS) {
        config.showInUI = _showInUI;
    }

    // write_to_file
    bool _writeToFile;
    if (root["write_to_file"].get(_writeToFile) == simdjson::SUCCESS) {
        config.writeToFile = _writeToFile;
    }

    // write_to_debug
    bool _writeToDebug;
    if (root["write_to_file"].get(_writeToDebug) == simdjson::SUCCESS) {
        config.writeToDebug = _writeToDebug;
    }

    // log_directory
    std::string_view _logDirectory;
    if (root["log_directory"].get(_logDirectory) == simdjson::SUCCESS) {
        std::string _logDirectoryStr = std::string(_logDirectory);
        juce::File dir(_logDirectoryStr);
        if (dir.isDirectory()) {
            config.logDirectory = dir;
        }
    }

    // log_file
    if (config.logDirectory.exists()) {
        juce::Time now = juce::Time::getCurrentTime();
        juce::String dateStr = now.formatted("%Y%m%d");
        juce::String timeStr = now.formatted("%H%M%S");
        juce::String levelStr = logLevelToString(config.logLevel);
        juce::String fileName = dateStr + "-" + timeStr + "-" + levelStr + ".log";
        config.logFile = config.logDirectory.getChildFile(fileName);
        if (!config.logFile.existsAsFile()) {
            if (!config.logFile.create().wasOk()) {
                config.logFile = juce::File();
                config.writeToFile = false;
            }
        }
    }

    return config;
}

// Returns platform default const config file
LoggingConfig LoggingConfigLoader::loadDefaultConfigFile() {
    return loadFromFile(getFileInSettingsFolder("log_settings_defaults.json"));
}


//AppLogger
bool AppLogger::initialised = false;
LoggingConfig AppLogger::currentConfig = {};

void AppLogger::initialise() {
    initialise(LoggingConfigLoader::load());
}

void AppLogger::initialise(LoggingConfig config) {
    if (initialised) { return; }
    bool isLoggerSet = false;

    if (config.writeToFile) {
        if (!config.logFile.existsAsFile()) {
            config.logFile.create();
        }
        if (config.logFile.hasWriteAccess()) {
            juce::Logger::setCurrentLogger(new juce::FileLogger(config.logFile, "Profiler Log", 10 * 1024 * 1024));
            isLoggerSet = true;
        }
    }
    if (!isLoggerSet) {
        juce::Logger::setCurrentLogger(nullptr);
    }
    currentConfig = config;
    initialised = true;
    AppLogger::info(LogCategory::INIT, "--==## Logger initialised ##==--");
}

void AppLogger::shutdown() {
    if (!initialised) { return; }
    AppLogger::info(LogCategory::INIT, "--==## Logger shutdown ##==--");
    delete juce::Logger::getCurrentLogger();
    juce::Logger::setCurrentLogger(nullptr);
    initialised = false;
}

bool AppLogger::isInitialised() noexcept {
    return initialised;
}

void AppLogger::log(LogLevel level, LogCategory category, const juce::String& message) {
    if (!initialised) { return; }
    if (level < currentConfig.logLevel) { return; }
    juce::String line;
    line << "[" << juce::Time::getCurrentTime().toString(true, true) << "] "
        << logLevelToString(level) << logCategoryToString(category) << message;
    #if JUCE_DEBUG
    DBG(line);
    #endif
    if (juce::Logger* logger = juce::Logger::getCurrentLogger()) {
        logger->writeToLog(line);
    }
}

void AppLogger::trace(LogCategory c, const juce::String& m) { log(LogLevel::TRACE,    c, m); }
void AppLogger::debug(LogCategory c, const juce::String& m) { log(LogLevel::DEBUG,    c, m); }
void AppLogger::info (LogCategory c, const juce::String& m) { log(LogLevel::INFO,     c, m); }
void AppLogger::warn (LogCategory c, const juce::String& m) { log(LogLevel::WARNING,  c, m); }
void AppLogger::error(LogCategory c, const juce::String& m) { log(LogLevel::ERROR,    c, m); }
void AppLogger::fatal(LogCategory c, const juce::String& m) { log(LogLevel::FATAL,    c, m); }