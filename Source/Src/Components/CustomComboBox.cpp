#include "Components/CustomComboBox.h"

#include "Stylesheet.h"

//=============================================================================
// CustomLookAndFeelLF Implementation
//=============================================================================

void CustomComboBoxLF::drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                                    int buttonX, int buttonY, int buttonW, int buttonH,
                                    juce::ComboBox& box) {
    auto area = juce::Rectangle<int>(0, 0, width, height).toFloat().reduced(1.0f);
    auto cornerSize = 4.0f;

    // Draw the background with a slight vertical gradient
    g.setGradientFill(ProfilerStyle::Gradients::vertical(
        area,
        ProfilerStyle::Colors::darkerGrey.brighter(0.2f),
        ProfilerStyle::Colors::darkerGrey.darker(0.2f),
        0.9f));
    g.fillRoundedRectangle(area, cornerSize);

    // Draw a very thin border
    g.setColour(ProfilerStyle::Colors::darkerGrey);
    g.drawRoundedRectangle(area, cornerSize, 1.0f);

    // Draw the small arrow on the right
    auto arrowZone = juce::Rectangle<int>(width - 25, 0, 20, height).toFloat();
    auto path = juce::Path();
    path.addTriangle(arrowZone.getCentreX() - 4.0f, arrowZone.getCentreY() - 2.0f,
                     arrowZone.getCentreX() + 4.0f, arrowZone.getCentreY() - 2.0f,
                     arrowZone.getCentreX(), arrowZone.getCentreY() + 4.0f);

    g.setColour(juce::Colours::white.withAlpha(0.7f));
    g.fillPath(path);
}

void CustomComboBoxLF::positionComboBoxText(juce::ComboBox& box, juce::Label& label) {
    label.setBounds(1, 1, box.getWidth() - 25, box.getHeight() - 2);
    label.setFont(juce::Font(16.0f));
    label.setJustificationType(juce::Justification::centred);
}

void CustomComboBoxLF::drawPopupMenuBackground(juce::Graphics& g, int width, int height) {
    g.fillAll(juce::Colour(0xff1a1a1a));
    g.setColour(juce::Colours::white.withAlpha(0.2f));
    g.drawRect(0, 0, width, height);
}

void CustomComboBoxLF::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                                         const bool isSeparator, const bool isActive,
                                         const bool isHighlighted, const bool isChecked,
                                         const bool hasSubMenu, const juce::String& text,
                                         const juce::String& shortcutKeyText,
                                         const juce::Drawable* icon, const juce::Colour* const textColourToUse) {
    auto r = area.toFloat().reduced(1.0f);

    // If the element is hovered (highlighted)
    if (isHighlighted) {
        g.setColour(juce::Colours::white.withAlpha(0.05f));
        g.fillRoundedRectangle(r, 3.0f);
    }

    // Text color
    g.setColour(isHighlighted ? juce::Colours::white : juce::Colours::white.withAlpha(0.7f));
    g.setFont(juce::Font(15.0f));

    // Text centered vertically and left aligned with some padding
    auto textRect = r.reduced(10.0f, 0);
    g.drawText(text, textRect, juce::Justification::centredLeft, true);

    // If the element is checked (the current profile)
    if (isChecked) {
        auto indicatorSize = r.getHeight() * 0.3f;
        auto indicatorRect = juce::Rectangle<float>(r.getRight() - 20.0f, r.getCentreY() - (indicatorSize * 0.5f),
                                                    indicatorSize, indicatorSize);
        g.setColour(ProfilerStyle::Colors::orange);
        g.fillEllipse(indicatorRect);
    }
}

//=============================================================================
// CustomComboBox Implementation
//=============================================================================

CustomComboBox::CustomComboBox() {
    setLookAndFeel(&_customLF);
}

CustomComboBox::~CustomComboBox() {
    setLookAndFeel(nullptr);
}

void CustomComboBox::paint(juce::Graphics& g) {
    juce::ComboBox::paint(g);
}

void CustomComboBox::resized() {
    juce::ComboBox::resized();
}