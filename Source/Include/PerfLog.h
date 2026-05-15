#pragma once
#include "Logging.h"

// ============================================================
//  PerfLog
//
//  Thin abstraction over the "_perf" Logger instance.
//  All calls compile to nothing unless ENABLE_PERF_LOG is
//  defined
//
//  Usage:
//      PerfLog::init(LogConfig::fromFile(perfConfigFile));
//      PerfLog::log("render took 4ms");
//      PerfLog::close();
//
//      PerfLog::Timer("thing being timed") // put at the start of function
//
// ============================================================

namespace PerfLog {

#ifdef ENABLE_PERF_LOG

static constexpr const char* LOGGER_NAME = "_perf";

inline void init(LogConfig config) {
    config.name = LOGGER_NAME;
    LogRegistry::create(LOGGER_NAME, std::move(config));
}

inline void log(const juce::String& message) {
    if (Logger* logger = LogRegistry::find(LOGGER_NAME))
        logger->log(LogLevel::Trace, LogCategory::Perf, message);
}

inline void close() {
    if (Logger* logger = LogRegistry::find(LOGGER_NAME))
        logger->shutdown();
}

class Timer {
   public:
    explicit Timer(const juce::String& str) : message(str) {
        this->logMsg("_start");
    }

    ~Timer() {
        this->logMsg("_end");
    }

    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

   private:
    void logMsg(const juce::String& suffix) const {
        const auto epochNs = juce::Time::getHighResolutionTicks();
        log(juce::String(epochNs) + suffix + " " + message);
    }

    const juce::String message;
};

#else

inline void init(LogConfig) {}           // compiled away
inline void log(const juce::String&) {}  // compiled away
inline void close() {}                   // compiled away

class Timer {
   public:
    explicit Timer(const juce::String&) {}
    ~Timer() = default;
    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;
};

#endif

}  // namespace PerfLog
