#include <JuceHeader.h>

#include "Components/CustomTabs.h"
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
    CustomTabs _eqDisplay;

    UtilityBarModule _utilityBar;

    MasterSlidersModule _masterSliders;
};