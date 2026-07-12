#pragma once
#include <JuceHeader.h>

namespace ProfilerStyle {
enum class Theme {
    Darker,
    Dark,
    Light,
    Orange
};

namespace Colors {
inline const juce::Colour white = juce::Colour(0xffffffff);

inline const juce::Colour lightestGrey  = juce::Colour(0xff505050);
inline const juce::Colour lighterGrey   = juce::Colour(0xff3a3a3a);
inline const juce::Colour lightGrey     = juce::Colour(0xff282828);
inline const juce::Colour grey          = juce::Colour(0xff252525);
inline const juce::Colour darkGrey      = juce::Colour(0xff323232);
inline const juce::Colour darkerGrey    = juce::Colour(0xff202020);
inline const juce::Colour darkestGrey   = juce::Colour(0xff171717);

inline const juce::Colour orange = juce::Colour(0xffff4500);
}  // namespace Colors

namespace Gradients {
inline static juce::ColourGradient vertical(
    juce::Rectangle<float> area,
    juce::Colour top,
    juce::Colour bottom,
    float midPoint = 0.5f) {
    juce::ColourGradient gradient(
        top, area.getX(), area.getY(),
        bottom, area.getX(), area.getBottom(),
        false);
    auto midColour = top.overlaidWith(bottom.withAlpha(0.5f));
    gradient.addColour(midPoint, bottom);

    return gradient;
}
}  // namespace Gradients
}  // namespace ProfilerStyle