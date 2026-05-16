#include "Components/CustomTabs.h"

CustomTabs::CustomTabs(std::vector<TabInfo> tabs, int defaultIndex) : _tabs(std::move(tabs)) {
    addAndMakeVisible(_tabBar);

    for (size_t i = 0; i < _tabs.size(); ++i) {
        _tabBar.addTab(_tabs[static_cast<size_t>(i)].name, juce::Colours::darkgrey, static_cast<int>(i));
    }

    _tabBar.onTabChanged = [this](int index) { changeView(index); };

    _tabBar.setCurrentTabIndex(defaultIndex);
}

void CustomTabs::paint(juce::Graphics& g) {}

void CustomTabs::resized() {
    auto area = getLocalBounds();

    auto tabBarHeight = getHeight() * 0.07f;                                   // 7% of the total height for the tab bar
    auto tabBarWidth = (tabBarHeight * 5) * static_cast<float>(_tabs.size());  // Calculate the total width needed for all tabs, each 5 times the tab bar height

    auto tabBarArea = area.removeFromTop(tabBarHeight)
                          .withSizeKeepingCentre(tabBarWidth, tabBarHeight);

    _tabBar.setBounds(tabBarArea);

    area.removeFromTop(5);

    if (_currentContent) {
        _currentContent->setBounds(area);
    }
}

void CustomTabs::changeView(int index) {
    if (index < 0 || index >= (int)_tabs.size()) return;

    _currentContent = _tabs[static_cast<size_t>(index)].createContent();

    if (_currentContent) {
        addAndMakeVisible(_currentContent.get());
        resized();
    }
}