#pragma once

#include <JuceHeader.h>

#include "PluginProcessor.h"

//===============================================================================
// CustomTabBar Implementation
//===============================================================================
class CustomTabBar : public juce::TabbedButtonBar {
   public:
    CustomTabBar(juce::TabbedButtonBar::Orientation orientation = juce::TabbedButtonBar::TabsAtTop);
    ~CustomTabBar() override = default;

    void resized() override;

    void currentTabChanged(int newCurrentTabIndex, const juce::String& newCurrentTabName) override;

    std::function<void(int)> onTabChanged;
};
