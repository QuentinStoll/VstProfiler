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
inline const juce::Colour background = juce::Colour(0xff0F0F12);
inline const juce::Colour container = juce::Colour(0xff1E1E24);
inline const juce::Colour elevated = juce::Colour(0xff1E1E24);
inline const juce::Colour border = juce::Colour(0xff2A2A35);
inline const juce::Colour text = juce::Colour(0xffE8E8ED);
inline const juce::Colour textMuted = juce::Colour(0xff8A8A96);
inline const juce::Colour caption = juce::Colour(0xff8A8A93);
inline const juce::Colour accent = juce::Colour(0xffFF5500);
inline const juce::Colour accentCyan = juce::Colour(0xff00E5FF);
inline const juce::Colour rigInput = juce::Colour(0xff10B981);
inline const juce::Colour rigAmp = juce::Colour(0xffFF5500);
inline const juce::Colour rigCab = juce::Colour(0xff3B82F6);
inline const juce::Colour rigEq = juce::Colour(0xffA855F7);

inline const juce::Colour white = text;
inline const juce::Colour lightestGrey = juce::Colour(0xff5C5C64);
inline const juce::Colour lighterGrey = juce::Colour(0xff3A3A40);
inline const juce::Colour lightGrey = border;
inline const juce::Colour grey = juce::Colour(0xff323238);
inline const juce::Colour darkGrey = border;
inline const juce::Colour darkerGrey = container;
inline const juce::Colour darkestGrey = background;
inline const juce::Colour orange = accent;

inline juce::Colour appBackground = background;
}  // namespace Colors

namespace Properties {
inline const juce::Identifier outlineVisible{"outlineVisible"};
inline const juce::Identifier outlineColour{"outlineColour"};
inline const juce::Identifier toggleLabelVisible{"toggleLabelVisible"};
}  // namespace Properties

namespace Fonts {
inline juce::String defaultFamily() {
    static const juce::String family = [] {
        const juce::StringArray preferred{"Inter", "SF Pro Text", "SF Pro Display", "Roboto"};
        const auto available = juce::Font::findAllTypefaceNames();

        for (const auto& name : preferred) {
            if (available.contains(name)) {
                return name;
            }
        }

#if JUCE_WINDOWS
        return juce::String("Segoe UI");
#elif JUCE_MAC
        return juce::String("Helvetica Neue");
#else
        return juce::Font::getDefaultSansSerifFontName();
#endif
    }();

    return family;
}

inline juce::Font regular(float height) {
    return juce::Font(juce::FontOptions(defaultFamily(), height, juce::Font::plain));
}

inline juce::Font bold(float height) {
    return juce::Font(juce::FontOptions(defaultFamily(), height, juce::Font::bold));
}
}  // namespace Fonts

namespace Surfaces {
inline constexpr float panelCorner = 6.0f;
inline constexpr float controlCorner = 6.0f;

inline void fillWindow(juce::Graphics& g, juce::Rectangle<float> area) {
    g.setColour(Colors::appBackground);
    g.fillRect(area);
}

inline void fillPanel(juce::Graphics& g, juce::Rectangle<float> area, float cornerRadius = panelCorner) {
    g.setColour(Colors::elevated);
    g.fillRoundedRectangle(area, cornerRadius);
    g.setColour(Colors::border);
    g.drawRoundedRectangle(area.reduced(0.5f), cornerRadius, 1.0f);
}
}  // namespace Surfaces

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
    gradient.addColour(midPoint, bottom);

    return gradient;
}
}  // namespace Gradients
}  // namespace ProfilerStyle
