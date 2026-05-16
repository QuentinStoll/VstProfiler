#include <JuceHeader.h>

#include "Components/CustomTabBar.h"
#include "PluginProcessor.h"

class CustomTabs : public juce::Component {
   public:
    CustomTabs(ProfilerAudioProcessor& p);
    ~CustomTabs();

    void paint(juce::Graphics& g) override;
    void resized() override;

   private:
    ProfilerAudioProcessor& _audioProcessor;

    CustomTabBar _tabBar;                              // Tab Bar
    std::unique_ptr<juce::Component> _currentContent;  // Pointer to the current content component (CloneView, PlayView, ProfilView)

    void changeView(int index);
};