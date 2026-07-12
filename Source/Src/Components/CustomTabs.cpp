#include "Components/CustomTabs.h"

CustomTabs::CustomTabs(std::vector<TabInfo> tabs, int defaultIndex) : _tabs(std::move(tabs)) {
    addAndMakeVisible(_tabBar);

    for (size_t i = 0; i < _tabs.size(); ++i) {
        _tabBar.addTab(_tabs[static_cast<size_t>(i)].name, juce::Colours::darkgrey, static_cast<int>(i));
    }

    _tabBar.onTabChanged = [this](int index) { changeView(index); };

    if (!_tabs.empty()) {
        const auto initialIndex = juce::jlimit(0, static_cast<int>(_tabs.size()) - 1, defaultIndex);
        _tabBar.setCurrentTabIndex(initialIndex);
        changeView(initialIndex);
    }
}

void CustomTabs::paint(juce::Graphics& /*g*/) {}

void CustomTabs::resized() {
    auto area = getLocalBounds();

    auto tabBarHeight = getHeight() * 0.05f;                                   // 5% of the total height for the tab bar
    auto tabBarWidth = (tabBarHeight * 5) * static_cast<float>(_tabs.size());  // Calculate the total width needed for all tabs, each 5 times the tab bar height

    area.removeFromTop(5);

    auto tabBarArea = area.removeFromTop(static_cast<int>(tabBarHeight))
                          .withSizeKeepingCentre(static_cast<int>(tabBarWidth), static_cast<int>(tabBarHeight));

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
