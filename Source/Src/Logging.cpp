#include "Logging.h"

#include <simdjson.h>
#include <stdexcept>

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

const char* toString(LogCategory category) noexcept {
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

} // namespace Log


//  LogConfig factory methods
LogConfig LogConfig::fromDefaultPath() {
    return fromFile(getFileInSettingsFolder("log_settings.json"));
}

LogConfig LogConfig::fromFile(const juce::File& file) {
    LogConfig config; // defaults set in struct declaration
    if (!file.existsAsFile()) return config;

    juce::String jsonText = file.loadFileAsString();
    if (jsonText.isEmpty()) return config;

    simdjson::dom::parser parser;
    simdjson::dom::element root;
    auto error =
        parser.parse(jsonText.toRawUTF8(), jsonText.getNumBytesAsUTF8())
            .get(root);
    if (error) return config;
    // name
    std::string_view name;
    if (root["name"].get(name) == simdjson::SUCCESS)
        config.name = std::string(name);
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
    bool showInUI = false;
    if (root["show_in_ui"].get(showInUI) == simdjson::SUCCESS)
        config.showInUI = showInUI;
    // write_to_file
    bool writeToFile = false;
    if (root["write_to_file"].get(writeToFile) == simdjson::SUCCESS)
        config.writeToFile = writeToFile;
    // write_to_debug
    bool writeToDebug = true;
    if (root["write_to_debug"].get(writeToDebug) == simdjson::SUCCESS)
        config.writeToDebug = writeToDebug;
    // log_directory
    std::string_view logDirectory;
    if (root["log_directory"].get(logDirectory) == simdjson::SUCCESS) {
        std::string logDirectoryStr = std::string(logDirectory);
        juce::File dir(logDirectoryStr);
        if (dir.isDirectory())
            config.logDirectory = dir;
    }
    // Derive log file path from directory
    if (config.logDirectory.exists()) {
        juce::Time now       = juce::Time::getCurrentTime();
        juce::String dateStr  = now.formatted("%Y%m%d");
        juce::String timeStr  = now.formatted("%H%M%S");
        juce::String fileName = dateStr + "-" + timeStr + "-" + config.name + ".log";
        config.logFile = config.logDirectory.getChildFile(fileName);
        if (!config.logFile.existsAsFile()) {
            if (!config.logFile.create().wasOk()) {
                config.logFile  = juce::File();
                config.writeToFile = false;
            }
        }
    }

    return config;
}

LogConfig LogConfig::fromDefaultConfigFile() {
    return fromFile(getFileInSettingsFolder("log_settings_defaults.json"));
}



//  LogRegistry
std::map<std::string, std::unique_ptr<Logger>> LogRegistry::registry_;

Logger& LogRegistry::create(const std::string& name, LogConfig config) {
    if (registry_.count(name))
        throw std::runtime_error("Logger '" + name + "' already exists. Use LogRegistry::get() instead.");

    config.name = name;
    auto instance = std::make_unique<Logger>(std::move(config));
    Logger& ref = *instance;
    registry_.emplace(name, std::move(instance));
    return ref;
}

Logger& LogRegistry::get(const std::string& name) {
    auto it = registry_.find(name);
    if (it == registry_.end())
        throw std::out_of_range("Logger '" + name + "' not found. Call LogRegistry::create() first.");
    return *it->second;
}

Logger* LogRegistry::find(const std::string& name) noexcept {
    auto it = registry_.find(name);
    return (it != registry_.end()) ? it->second.get() : nullptr;
}

void LogRegistry::shutdownAll() {
    for (auto& [name, instance] : registry_)
        instance->shutdown();
    registry_.clear();
}



//  Logger instance
Logger::Logger(LogConfig config)
    : config_(std::move(config))
{
    initialise();
}

Logger::~Logger() {
    shutdown();
}

void Logger::initialise() {
    if (initialised_) return;

    if (config_.writeToFile) {
        if (!config_.logFile.existsAsFile())
            config_.logFile.create();
        if (config_.logFile.hasWriteAccess()) {
            fileStream_ = std::make_unique<juce::FileOutputStream>(config_.logFile);
            if (fileStream_->failedToOpen()) {
                fileStream_.reset();
                config_.writeToFile = false;
            }
        }
    }

    initialised_ = true;
    info(LogCategory::Init, "Logger '" + juce::String(config_.name) + "' initialised");

    if (fileStream_ != nullptr)
        info(LogCategory::Init, "Writing to file: " + config_.logFile.getFullPathName());
    else if (config_.writeToFile)
        info(LogCategory::Init, "File logging requested but file could not be opened\n falling back to debug output only.");
}

void Logger::shutdown() {
    if (!initialised_) return;

    info(LogCategory::Init, "Logger '" + juce::String(config_.name) + "' shutdown");
    fileStream_.reset();
    initialised_ = false;
}

bool Logger::isInitialised() const noexcept {
    return initialised_;
}

const LogConfig& Logger::getConfig() const noexcept {
    return config_;
}

void Logger::reloadConfig() {
    shutdown();
    config_ = LogConfig::fromDefaultPath();
    initialise();
}

void Logger::log(LogLevel level, LogCategory category, const juce::String& message) {
    if (!initialised_)              return;
    if (level < config_.logLevel)   return;

    juce::String line;
    line << "[" << juce::Time::getCurrentTime().toString(true, true) << "] "
         << "[" << config_.name << "] "
         << Log::toString(level) << " " << Log::toString(category) << " " << message;

    #if JUCE_DEBUG
    if (config_.writeToDebug)
        DBG(line);
    #endif

    if (fileStream_) {
        fileStream_->writeText(line + "\n", false, false, nullptr);
        fileStream_->flush();
    }
}


void Logger::trace(LogCategory c, const juce::String& m) {
    log(LogLevel::Trace, c, m);
}
void Logger::debug(LogCategory c, const juce::String& m) {
    log(LogLevel::Debug, c, m);
}
void Logger::info(LogCategory c, const juce::String& m) {
    log(LogLevel::Info, c, m);
}
void Logger::warn(LogCategory c, const juce::String& m) {
    log(LogLevel::Warning, c, m);
}
void Logger::error(LogCategory c, const juce::String& m) {
    log(LogLevel::Error, c, m);
}
void Logger::fatal(LogCategory c, const juce::String& m) {
    log(LogLevel::Fatal, c, m);
}
