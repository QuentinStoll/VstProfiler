#pragma once

#include <JuceHeader.h>

#include "SignalChainLayout.h"

namespace Fx {
enum class ParamType { Float, Bool, Choice };

struct ParamSpec {
    const char* id;
    const char* label;
    ParamType type;
    float minimum;
    float maximum;
    float step;
    float defaultValue;
    float skew;
    const char* suffix;
    const char* choices;
};

struct ModuleSpec {
    SignalChain::Stage stage;
    const char* name;
    const char* caption;
    const char* algorithm;
    const ParamSpec* params;
    int paramCount;

    const char* bypassId() const noexcept { return paramCount > 0 ? params[0].id : ""; }
};

int moduleCount() noexcept;
const ModuleSpec& moduleAt(int index) noexcept;
const ModuleSpec* moduleFor(SignalChain::Stage stage) noexcept;
int parameterCount() noexcept;
const char* captionFor(SignalChain::Stage stage) noexcept;
void addParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout);
}  // namespace Fx
