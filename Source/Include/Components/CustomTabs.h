#pragma once

#include <JuceHeader.h>

#include <vector>

#include "Components/CustomTabBar.h"
#include "PluginProcessor.h"

class CustomTabs : public juce::Component {
   public:
    struct TabInfo {
        juce::String name;
        std::function<std::unique_ptr<juce::Component>()> createContent;
    };

    explicit CustomTabs(std::vector<TabInfo> tabs, int defaultIndex = 0);
    ~CustomTabs() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void changeView(int index);

   private:
    std::vector<TabInfo> _tabs;                        // List of tabs with their names and content creation functions
    CustomTabBar _tabBar;                              // Tab Bar
    std::unique_ptr<juce::Component> _currentContent;  // Pointer to the current content component (CloneView, PlayView, ProfilView)
};