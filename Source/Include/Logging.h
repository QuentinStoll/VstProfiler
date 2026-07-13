#pragma once
#include <JuceHeader.h>

#include <map>
#include <memory>
#include <string>

enum class LogLevel { Trace,
                      Debug,
                      Info,
                      Warning,
                      Error,
                      Fatal,
                      Other };

enum class LogCategory { Init,
                         Dsp,
                         Io,
                         Ui,
                         Param,
                         Host,
                         Perf,
                         Other };

//  Log helpers
namespace Log {
const char* toString(LogLevel level) noexcept;
const char* toString(LogCategory category) noexcept;
}  // namespace Log

//  LogConfig
struct LogConfig {
    std::string name = "default";
    LogLevel logLevel = LogLevel::Info;
    bool showInUI = false;
    bool writeToFile = false;
    bool writeToDebug = true;
    bool writeToTracy = true;
    juce::File logDirectory;
    juce::File logFile;

    static LogConfig fromDefaultPath();
    static LogConfig fromFile(const juce::File& file);
    static LogConfig fromDefaultConfigFile();
};

//  Logger
class Logger final {
   public:
    explicit Logger(LogConfig config);
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void shutdown();
    bool isInitialised() const noexcept;
    const LogConfig& getConfig() const noexcept;
    void reloadConfig();

    void log(LogLevel level, LogCategory category, const juce::String& message);
    void trace(LogCategory category, const juce::String& message);
    void debug(LogCategory category, const juce::String& message);
    void info(LogCategory category, const juce::String& message);
    void warn(LogCategory category, const juce::String& message);
    void error(LogCategory category, const juce::String& message);
    void fatal(LogCategory category, const juce::String& message);

   private:
    void initialise();

    LogConfig config_;
    bool initialised_ = false;
    std::unique_ptr<juce::FileOutputStream> fileStream_;
};

//  LogRegistry
class LogRegistry final {
   public:
    static Logger& create(const std::string& name, LogConfig config);
    static Logger& get(const std::string& name);
    static Logger* find(const std::string& name) noexcept;
    static void shutdownAll();

   private:
    LogRegistry() = delete;
    static std::map<std::string, std::unique_ptr<Logger>> registry_;
};