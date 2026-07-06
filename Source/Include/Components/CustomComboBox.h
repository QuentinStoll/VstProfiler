#pragma once

#include <JuceHeader.h>

//=============================================================================
// Custom LookAndFeel for the ComboBox
//=============================================================================

class CustomComboBoxLF : public juce::LookAndFeel_V4 {
   public:
    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox& box) override;

    void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override;

    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override;

    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                           const bool isSeparator, const bool isActive,
                           const bool isHighlighted, const bool isChecked,
                           const bool hasSubMenu, const juce::String& text,
                           const juce::String& shortcutKeyText,
                           const juce::Drawable* icon, const juce::Colour* const textColourToUse) override;

   private:
};

//=============================================================================
// Custom ComboBox component that uses the CustomComboBoxLF
//=============================================================================

class CustomComboBox : public juce::ComboBox {
   public:
    CustomComboBox();
    ~CustomComboBox();

    void paint(juce::Graphics& g) override;
    void resized() override;

   private:
    CustomComboBoxLF _customLF;
};
