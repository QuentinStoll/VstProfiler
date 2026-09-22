#include "Components/CustomTextEditor.h"

#include "Stylesheet.h"

CustomTextEditor::CustomTextEditor() {
    setMultiLine(false);
    setReturnKeyStartsNewLine(false);
    setScrollbarsShown(false);
    setSelectAllWhenFocused(true);
    setJustification(juce::Justification::centredLeft);
    setIndents(8, 0);
    setColour(juce::TextEditor::backgroundColourId, ProfilerStyle::Colors::elevated);
    setColour(juce::TextEditor::textColourId, ProfilerStyle::Colors::text);
    setColour(juce::TextEditor::outlineColourId, ProfilerStyle::Colors::darkGrey);
    setColour(juce::TextEditor::focusedOutlineColourId, ProfilerStyle::Colors::accent);
    setColour(juce::TextEditor::highlightColourId, ProfilerStyle::Colors::accent.withAlpha(0.28f));
    setColour(juce::TextEditor::highlightedTextColourId, ProfilerStyle::Colors::text);
    setFont(ProfilerStyle::Fonts::regular(15.0f));
}
