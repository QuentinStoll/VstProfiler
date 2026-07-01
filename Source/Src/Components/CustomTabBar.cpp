#include "Components/CustomTabBar.h"

#include "Stylesheet.h"

//===============================================================================
// CustomTabBarLF Implementation
//===============================================================================

void CustomTabBarLF::drawTabbedButtonBarBackground(juce::TabbedButtonBar& bar, juce::Graphics& g) {
    const float cornerSize = 3.0f;

    g.setColour(ProfilerStyle::Colors::darkerGrey);
    g.fillRoundedRectangle(0.f, 0.f,
                           static_cast<float>(bar.getWidth()),
                           static_cast<float>(bar.getHeight()),
                           cornerSize);
}

void CustomTabBarLF::drawTabButton(juce::TabBarButton& button,
                                   juce::Graphics& g,
                                   bool isMouseOver,
                                   bool isMouseDown) {
    auto area = button.getLocalBounds().toFloat().reduced(2.0f);
    auto buttonColour = button.isFrontTab() ? ProfilerStyle::Colors::darkerGrey : ProfilerStyle::Colors::darkGrey;

    if (isMouseOver && !isMouseDown) buttonColour = buttonColour.brighter(0.1f);

    g.setGradientFill(ProfilerStyle::Gradients::vertical(
        area,
        buttonColour.brighter(0.2f),
        buttonColour.darker(0.2f),
        0.9f));

    g.fillRoundedRectangle(area, 2.0f);

    g.setColour(juce::Colours::white);
    g.setFont(14.0f);

    g.drawText(button.getButtonText(), area, juce::Justification::centred);
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