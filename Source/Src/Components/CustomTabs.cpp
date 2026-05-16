#include "Components/CustomTabs.h"

#include "Views/CloneView.h"
#include "Views/PlayView.h"
#include "Views/ProfilView.h"

CustomTabs::CustomTabs(ProfilerAudioProcessor& p) : _audioProcessor(p) {
    // setLookAndFeel(&_lookAndFeel);

    addAndMakeVisible(_tabBar);

    _tabBar.addTab("Clone", juce::Colours::darkgrey, 0);
    _tabBar.addTab("Play", juce::Colours::darkgrey, 1);
    _tabBar.addTab("Profil", juce::Colours::darkgrey, 2);

    _tabBar.onTabChanged = [this](int index) { changeView(index); };

    _tabBar.setCurrentTabIndex(1);  // Set the initial tab index to 1 (Play)
}

CustomTabs::~CustomTabs() {
    setLookAndFeel(nullptr);
}

void CustomTabs::paint(juce::Graphics& g) {}

void CustomTabs::resized() {
    auto area = getLocalBounds();

    auto tabBarHeight = getHeight() * 0.07f;    // 7% of the total height for the tab bar
    auto tabBarWidth = (tabBarHeight * 5) * 3;  // Calculate the total width needed for 3 tabs, each 5 times the tab bar height

    auto tabBarArea = area.removeFromTop(tabBarHeight)
                          .withSizeKeepingCentre(tabBarWidth, tabBarHeight);

    _tabBar.setBounds(tabBarArea);

    area.removeFromTop(5);

    if (_currentContent) {
        _currentContent->setBounds(area);
    }
}

void CustomTabs::changeView(int index) {
    _currentContent = nullptr;  // Clear the current content before creating a new one

    switch (index) {
        case 0:
            _currentContent = std::make_unique<CloneView>(_audioProcessor);
            break;
        case 1:
            _currentContent = std::make_unique<PlayView>(_audioProcessor);
            break;
        case 2:
            _currentContent = std::make_unique<ProfilView>(_audioProcessor);
            break;
        default:
            break;
    }

    if (_currentContent) {
        addAndMakeVisible(_currentContent.get());
        resized();
    }
}