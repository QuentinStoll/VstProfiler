#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ScreenID.h"

class ViewManager : public juce::Component
{
public:
    ViewManager(ProfilerAudioProcessor&);
    ~ViewManager();

	void resized() override;
	void changeView(ScreenID screenID);

private:
	std::unique_ptr<juce::Component> _currentView;
    juce::TextButton _backButton{ "Back" };

    ProfilerAudioProcessor& _audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ViewManager)
};