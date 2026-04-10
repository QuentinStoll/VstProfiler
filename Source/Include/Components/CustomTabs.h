#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Styles/CustomTabsLF.h"
#include "Components/CustomTabBar.h"

class CustomTabs : public juce::Component
{
public:
	CustomTabs(ProfilerAudioProcessor& p);
	~CustomTabs();

	void paint(juce::Graphics& g) override;
	void resized() override;

private:
	CustomTabsLF _lookAndFeel; // Look and Feel for the tabs
	ProfilerAudioProcessor& _audioProcessor;

	CustomTabBar _tabBar; // Tab Bar
	std::unique_ptr<juce::Component> _currentContent; // Pointer to the current content component (CloneView, PlayView, ProfilView)

	void changeView(int index);
};