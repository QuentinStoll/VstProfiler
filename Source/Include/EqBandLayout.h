#pragma once

#include <JuceHeader.h>

namespace EqBands {
constexpr int count = 6;
constexpr float minHz = 20.0f;
constexpr float maxHz = 20000.0f;
constexpr float minDb = -24.0f;
constexpr float maxDb = 24.0f;
constexpr float peakQ = 1.0f;
constexpr float shelfQ = 0.707f;

enum class Type {
    LowShelf,
    Peak,
    HighShelf
};

struct Spec {
    const char* gainId;
    const char* freqId;
    const char* label;
    float defaultHz;
    float defaultDb;
    Type type;
};

inline constexpr Spec specs[count] = {
    {"depth", "depthFreq", "LS", 80.0f, 2.5f, Type::LowShelf},
    {"bass", "bassFreq", "B1", 180.0f, 1.0f, Type::Peak},
    {"mid", "midFreq", "B2", 450.0f, -3.5f, Type::Peak},
    {"highMid", "highMidFreq", "B3", 1000.0f, 1.5f, Type::Peak},
    {"treble", "trebleFreq", "B4", 2800.0f, 3.0f, Type::Peak},
    {"presence", "presenceFreq", "HS", 6000.0f, -1.5f, Type::HighShelf},
};

inline juce::NormalisableRange<float> freqRange() {
    return {minHz, maxHz, 0.1f, 0.25f};
}

inline juce::NormalisableRange<float> gainRange() {
    return {minDb, maxDb, 0.1f};
}
}  // namespace EqBands
