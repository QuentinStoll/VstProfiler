#pragma once

#include <JuceHeader.h>

class CustomTextEditor : public juce::TextEditor {
public:
    CustomTextEditor();
    ~CustomTextEditor() override;

private:
    class CustomTextEditorLF : public juce::LookAndFeel_V4 {
    public:
        void fillTextEditorBackground(juce::Graphics& g, int width, int height, juce::TextEditor& textEditor) override {
            g.setColour(textEditor.findColour(juce::TextEditor::backgroundColourId));
            g.fillRoundedRectangle(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 6.0f);
        }

        void drawTextEditorOutline(juce::Graphics& g, int width, int height, juce::TextEditor& textEditor) override {
            auto borderColour = textEditor.hasKeyboardFocus(true) 
                                ? textEditor.findColour(juce::TextEditor::focusedOutlineColourId) 
                                : textEditor.findColour(juce::TextEditor::outlineColourId);
            
            g.setColour(borderColour);
            auto area = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
            g.drawRoundedRectangle(area.reduced(0.5f), 6.0f, 1.0f);
        }
    };

    CustomTextEditorLF _customLF;
};