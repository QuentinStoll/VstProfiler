#include <JuceHeader.h>

#include "Components/CustomTabBar.h"
#include "Modules/MasterSlidersModule.h"
#include "Modules/UtilityBarModule.h"
#include "PluginProcessor.h"

class PlayView : public juce::Component {
   public:
    PlayView(ProfilerAudioProcessor& p);
    ~PlayView();

    void paint(juce::Graphics& g) override;
    void resized() override;

   private:
    ProfilerAudioProcessor& _audioProcessor;

    CustomTabBar _eqDisplayBar;

    UtilityBarModule _utilityBar;

    std::unique_ptr<juce::Component> _currentContent;

    void changeEqModule(int index);

    MasterSlidersModule _masterSliders;
};