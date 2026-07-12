#include "Components/CustomTextEditor.h"

#include "Stylesheet.h"

CustomTextEditor::CustomTextEditor() {
    setLookAndFeel(&_customLF);

    setMultiLine(false);
    setReturnKeyStartsNewLine(false);
    setScrollbarsShown(false);
    setSelectAllWhenFocused(true);
    setJustification(juce::Justification::centredLeft);
    setIndents(12, 0); 

    setColour(juce::TextEditor::backgroundColourId, ProfilerStyle::Colors::darkGrey);
    setColour(juce::TextEditor::textColourId, ProfilerStyle::Colors::white);
    setColour(juce::TextEditor::outlineColourId, ProfilerStyle::Colors::darkerGrey);
    setColour(juce::TextEditor::focusedOutlineColourId, ProfilerStyle::Colors::orange);
    setColour(juce::TextEditor::highlightColourId, ProfilerStyle::Colors::orange.withAlpha(0.35f));
    setColour(juce::TextEditor::highlightedTextColourId, ProfilerStyle::Colors::white);
}

CustomTextEditor::~CustomTextEditor() {
    setLookAndFeel(nullptr);
}