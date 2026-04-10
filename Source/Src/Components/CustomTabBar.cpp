#include "Components/CustomTabBar.h"

void CustomTabBar::currentTabChanged(int newCurrentTabIndex, const juce::String& newCurrentTabName)
{
	DBG("Current tab changed to index: " << newCurrentTabIndex << ", name: " << newCurrentTabName);
	if (onTabChanged) {
		onTabChanged(newCurrentTabIndex);
	}
}