#include <simdjson.h>
#include <string_view>
#include <stdexcept>
#include "Logging.h"
#include "SettingsPath.h"
#include "juce_core/juce_core.h"



//  Log namespace helpers
namespace Log {

const char* toString(LogLevel level) noexcept {
    switch (level) {
        case LogLevel::TRACE:   return "[TRACE]";
        case LogLevel::DEBUG:   return "[DEBUG]";
        case LogLevel::INFO:    return "[INFO]";
        case LogLevel::WARNING: return "[WARNING]";
        case LogLevel::ERROR:   return "[ERROR]";
        case LogLevel::FATAL:   return "[FATAL]";
        default:                return "[OTHER]";
    }
}

const char* toString(LogCategory category) noexcept {
    switch (category) {
        case LogCategory::INIT:  return "[INIT]";
        case LogCategory::DSP:   return "[DSP]";
        case LogCategory::IO:    return "[IO]";
        case LogCategory::UI:    return "[UI]";
        case LogCategory::PARAM: return "[PARAM]";
        case LogCategory::HOST:  return "[HOST]";
        case LogCategory::PERF:  return "[PERF]";
        default:                 return "[OTHER]";
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
    auto error = parser.parse(jsonText.toRawUTF8(), jsonText.getNumBytesAsUTF8()).get(root);
    if (error) return config;
    // name
    std::string_view name;
    if (root["name"].get(name) == simdjson::SUCCESS)
        config.name = std::string(name);
    // log_level
    std::string_view logLevel;
    if (root["log_level"].get(logLevel) == simdjson::SUCCESS) {
        if      (logLevel == "TRACE")   { config.logLevel = LogLevel::TRACE;   }
        else if (logLevel == "DEBUG")   { config.logLevel = LogLevel::DEBUG;   }
        else if (logLevel == "INFO")    { config.logLevel = LogLevel::INFO;    }
        else if (logLevel == "WARNING") { config.logLevel = LogLevel::WARNING; }
        else if (logLevel == "ERROR")   { config.logLevel = LogLevel::ERROR;   }
        else if (logLevel == "FATAL")   { config.logLevel = LogLevel::FATAL;   }
        else                            { config.logLevel = LogLevel::OTHER;   }
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
    info(LogCategory::INIT, "Logger '" + juce::String(config_.name) + "' initialised");

    if (fileStream_ != nullptr)
        info(LogCategory::INIT, "Writing to file: " + config_.logFile.getFullPathName());
    else if (config_.writeToFile)
        info(LogCategory::INIT, "File logging requested but file could not be opened\n falling back to debug output only.");
}

void Logger::shutdown() {
    if (!initialised_) return;

    info(LogCategory::INIT, "Logger '" + juce::String(config_.name) + "' shutdown");
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

void Logger::trace(LogCategory c, const juce::String& m) { log(LogLevel::TRACE,   c, m); }
void Logger::debug(LogCategory c, const juce::String& m) { log(LogLevel::DEBUG,   c, m); }
void Logger::info (LogCategory c, const juce::String& m) { log(LogLevel::INFO,    c, m); }
void Logger::warn (LogCategory c, const juce::String& m) { log(LogLevel::WARNING, c, m); }
void Logger::error(LogCategory c, const juce::String& m) { log(LogLevel::ERROR,   c, m); }
void Logger::fatal(LogCategory c, const juce::String& m) { log(LogLevel::FATAL,   c, m); }