#include "Logging.h"

#include <simdjson.h>

#include <string_view>

#include "SettingsPath.h"
#include "juce_core/juce_core.h"

const char* logLevelToString(LogLevel level) noexcept {
    switch (level) {
        case LogLevel::Trace:
            return "[Trace]";
        case LogLevel::Debug:
            return "[Debug]";
        case LogLevel::Info:
            return "[Info]";
        case LogLevel::Warning:
            return "[Warning]";
        case LogLevel::Error:
            return "[Error]";
        case LogLevel::Fatal:
            return "[Fatal]";
        default:
            return "[Other]";
    }
}

const char* logCategoryToString(LogCategory category) noexcept {
    switch (category) {
        case LogCategory::Init:
            return "[Init]";
        case LogCategory::Dsp:
            return "[Dsp]";
        case LogCategory::Io:
            return "[Io]";
        case LogCategory::Ui:
            return "[Ui]";
        case LogCategory::Param:
            return "[Param]";
        case LogCategory::Host:
            return "[Host]";
        case LogCategory::Perf:
            return "[Perf]";
        default:
            return "[Other]";
    }
}

// LoggingConfigLoader
//  Load config from default platform path
LoggingConfig LoggingConfigLoader::load() {
    return loadFromFile(getFileInSettingsFolder("log_settings.json"));
}

// Load config from a specific file
LoggingConfig LoggingConfigLoader::loadFromFile(const juce::File& file) {
    LoggingConfig config;  // defaults already set in struct
    if (!file.existsAsFile()) return config;
    juce::String jsonText = file.loadFileAsString();
    if (jsonText.isEmpty()) return config;
    simdjson::dom::parser parser;
    simdjson::dom::element root;
    auto error =
        parser.parse(jsonText.toRawUTF8(), jsonText.getNumBytesAsUTF8())
            .get(root);
    if (error) return config;

    // log_level
    std::string_view _logLevel;
    if (root["log_level"].get(_logLevel) == simdjson::SUCCESS) {
        if (_logLevel == "Trace") {
            config.logLevel = LogLevel::Trace;
        } else if (_logLevel == "Debug") {
            config.logLevel = LogLevel::Debug;
        } else if (_logLevel == "Info") {
            config.logLevel = LogLevel::Info;
        } else if (_logLevel == "Warning") {
            config.logLevel = LogLevel::Warning;
        } else if (_logLevel == "Error") {
            config.logLevel = LogLevel::Error;
        } else if (_logLevel == "Fatal") {
            config.logLevel = LogLevel::Fatal;
        } else {
            config.logLevel = LogLevel::Other;
        }
    }

    // show_in_ui
    bool _showInUi;
    if (root["show_in_ui"].get(_showInUi) == simdjson::SUCCESS) {
        config.showInUi = _showInUi;
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
        juce::String fileName =
            dateStr + "-" + timeStr + "-" + levelStr + ".log";
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

// AppLogger
bool AppLogger::initialised = false;
LoggingConfig AppLogger::currentConfig = {};

void AppLogger::initialise() { initialise(LoggingConfigLoader::load()); }

void AppLogger::initialise(LoggingConfig config) {
    if (initialised) {
        return;
    }
    bool isLoggerSet = false;

    if (config.writeToFile) {
        if (!config.logFile.existsAsFile()) {
            config.logFile.create();
        }
        if (config.logFile.hasWriteAccess()) {
            juce::Logger::setCurrentLogger(new juce::FileLogger(
                config.logFile, "Profiler Log", 10 * 1024 * 1024));
            isLoggerSet = true;
        }
    }
    if (!isLoggerSet) {
        juce::Logger::setCurrentLogger(nullptr);
    }
    currentConfig = config;
    initialised = true;
    AppLogger::info(LogCategory::Init, "--==## Logger initialised ##==--");
}

void AppLogger::shutdown() {
    if (!initialised) {
        return;
    }
    AppLogger::info(LogCategory::Init, "--==## Logger shutdown ##==--");
    delete juce::Logger::getCurrentLogger();
    juce::Logger::setCurrentLogger(nullptr);
    initialised = false;
}

bool AppLogger::isInitialised() noexcept { return initialised; }

void AppLogger::log(LogLevel level, LogCategory category,
                    const juce::String& message) {
    if (!initialised) {
        return;
    }
    if (level < currentConfig.logLevel) {
        return;
    }
    juce::String line;
    line << "[" << juce::Time::getCurrentTime().toString(true, true) << "] "
         << logLevelToString(level) << logCategoryToString(category) << message;
#if JUCE_Debug
    DBG(line);
#endif
    if (juce::Logger* logger = juce::Logger::getCurrentLogger()) {
        logger->writeToLog(line);
    }
}

void AppLogger::trace(LogCategory c, const juce::String& m) {
    log(LogLevel::Trace, c, m);
}
void AppLogger::debug(LogCategory c, const juce::String& m) {
    log(LogLevel::Debug, c, m);
}
void AppLogger::info(LogCategory c, const juce::String& m) {
    log(LogLevel::Info, c, m);
}
void AppLogger::warn(LogCategory c, const juce::String& m) {
    log(LogLevel::Warning, c, m);
}
void AppLogger::error(LogCategory c, const juce::String& m) {
    log(LogLevel::Error, c, m);
}
void AppLogger::fatal(LogCategory c, const juce::String& m) {
    log(LogLevel::Fatal, c, m);
}