#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class CustomTabBar : public juce::TabbedButtonBar
{
public:
	CustomTabBar() : juce::TabbedButtonBar(juce::TabbedButtonBar::TabsAtTop) {}
	~CustomTabBar() {};

	void currentTabChanged(int newCurrentTabIndex, const juce::String& newCurrentTabName) override;

	std::function<void(int)> onTabChanged; // Callback for tab change events

private:

};