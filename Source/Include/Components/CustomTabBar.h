#pragma once

#include <JuceHeader.h>

#include "PluginProcessor.h"

//===============================================================================
// CustomTabBarLF Implementation
//===============================================================================

class CustomTabBarLF : public juce::LookAndFeel_V4 {
   public:
    void drawTabbedButtonBarBackground(juce::TabbedButtonBar& bar, juce::Graphics& g) override;

    void drawTabButton(juce::TabBarButton& button,
                       juce::Graphics& g,
                       bool isMouseOver,
                       bool isMouseDown) override;

    void drawTabAreaBehindFrontButton(juce::TabbedButtonBar& bar, juce::Graphics& g, int w, int h) override {}

    int getTabButtonBestWidth(juce::TabBarButton& bar, int tabDepth) override { return tabDepth; }
};

//===============================================================================
// CustomTabBar Implementation
//===============================================================================
class CustomTabBar : public juce::TabbedButtonBar {
   public:
    CustomTabBar(juce::TabbedButtonBar::Orientation orientation = juce::TabbedButtonBar::TabsAtTop);
    ~CustomTabBar();

    void paint(juce::Graphics& g) override;
    void resized() override;

    void currentTabChanged(int newCurrentTabIndex, const juce::String& newCurrentTabName) override;

    std::function<void(int)> onTabChanged;  // Callback for tab change events

   private:
    CustomTabBarLF _customLF;
};