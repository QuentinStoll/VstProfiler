#include <JuceHeader.h>

#include "Components/CustomTextButton.h"
#include "PluginProcessor.h"

class CloneView : public juce::Component {
   public:
    CloneView(ProfilerAudioProcessor& p);
    ~CloneView();

    void paint(juce::Graphics& g) override;
    void resized() override;

   private:
    ProfilerAudioProcessor& _audioProcessor;

    CustomTextButton _sweepButton{"Start Sweep"};
    CustomTextButton _loadIRButton{"Load IR"};
    CustomTextButton _loadAmpButton{"Load Amp"};
};
