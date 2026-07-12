#include "Components/CustomTabBar.h"

#include "Stylesheet.h"

//===============================================================================
// CustomTabBarLF Implementation
//===============================================================================

void CustomTabBarLF::drawTabbedButtonBarBackground(juce::TabbedButtonBar& bar, juce::Graphics& g) {
    const float cornerSize = 3.0f;

    g.setColour(ProfilerStyle::Colors::darkestGrey);
    g.fillRoundedRectangle(0.f, 0.f,
                           static_cast<float>(bar.getWidth()),
                           static_cast<float>(bar.getHeight()),
                           cornerSize);
}

void CustomTabBarLF::drawTabButton(juce::TabBarButton& button,
                                   juce::Graphics& g,
                                   bool isMouseOver,
                                   bool isMouseDown)
{
    auto slotArea = button.getLocalBounds().toFloat().reduced(2.0f, 4.0f);
    const bool isFullyPressed = isMouseDown || button.isFrontTab();
    
    const auto fullyPressedColour = ProfilerStyle::Colors::darkerGrey; 
    const auto normalColour       = ProfilerStyle::Colors::darkGrey;
    const auto hoverColour        = normalColour.darker(0.2f); 

    auto faceColour = normalColour;
    if (isFullyPressed)       faceColour = fullyPressedColour;
    else if (isMouseOver) faceColour = hoverColour;

    // Mechanics & translation
    const float maxTravel = 4.0f; 
    auto faceArea = slotArea; 

    float yOffset = 0.0f;
    if (!isFullyPressed) {
        if (isMouseOver) {
            yOffset = -maxTravel * 0.5f; 
        } else {
            yOffset = -maxTravel; 
        }
    }

    faceArea = faceArea.translated(0.0f, yOffset);

    // Draw slot background
    if (!isFullyPressed) {
        g.setColour(fullyPressedColour);
        g.fillRoundedRectangle(slotArea, 2.0f);
    }

    // Draw button face
    g.setGradientFill(ProfilerStyle::Gradients::vertical(
        faceArea,
        faceColour,
        faceColour.darker(0.12f),
        1.0f));

    g.fillRoundedRectangle(faceArea, 2.0f);

    // Top highlight
    if (!isFullyPressed) {
        g.setColour(faceColour.brighter(0.25f).withAlpha(0.4f));
        g.drawRoundedRectangle(faceArea, 2.0f, 1.0f);
    }

    // Draw text with context-based colors
    auto textColour = isFullyPressed ? juce::Colours::white : juce::Colour(0xffafafaf);
    g.setColour(textColour.withAlpha(isFullyPressed ? 1.0f : 0.8f));
    g.setFont(14.0f);
    g.drawText(button.getButtonText(), faceArea, juce::Justification::centred);
}

//===============================================================================
// CustomTabBar Implementation
//===============================================================================

CustomTabBar::CustomTabBar(juce::TabbedButtonBar::Orientation orientation)
    : juce::TabbedButtonBar(orientation) {
    setLookAndFeel(&_customLF);
}

CustomTabBar::~CustomTabBar() {
    setLookAndFeel(nullptr);
}

void CustomTabBar::paint(juce::Graphics& g) {
    juce::TabbedButtonBar::paint(g);
}

void CustomTabBar::resized() {
    juce::TabbedButtonBar::resized();

    const int numTabs = getNumTabs();
    if (numTabs == 0) return;

    const int tabW = getWidth() / numTabs;
    const int tabH = getHeight();

    for (int i = 0; i < numTabs; ++i) {
        if (auto* btn = getTabButton(i))
            btn->setBounds(tabW * i, 0, tabW, tabH);
    }
}

void CustomTabBar::currentTabChanged(int newCurrentTabIndex, const juce::String& newCurrentTabName) {
    DBG("Current tab changed to index: " << newCurrentTabIndex << ", name: " << newCurrentTabName);
    if (onTabChanged) {
        onTabChanged(newCurrentTabIndex);
    }
}