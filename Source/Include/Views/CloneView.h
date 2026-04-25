#include "JuceHeader.h"
#include "PluginProcessor.h"
#include "Styles/CustomButtonLF.h"

class CloneView : public juce::Component {
   public:
    CloneView(ProfilerAudioProcessor& p);
    ~CloneView();

    void paint(juce::Graphics& g) override;
    void resized() override;

   private:
    CustomButtonLF _lookAndFeel;
    ProfilerAudioProcessor& _audioProcessor;

    juce::TextButton _sweepButton{"Start Sweep"};
    juce::TextButton _loadIRButton{"Load IR"};
    juce::TextButton _loadAmpButton{"Load Amp"};
};
