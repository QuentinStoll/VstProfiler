#include <JuceHeader.h>

#include "Components/CustomComboBox.h"
#include "Components/CustomTextButton.h"
#include "Components/CustomToggleButton.h"
#include "Stylesheet.h"

class UtilityBarModule : public juce::Component {
   public:
    UtilityBarModule(juce::AudioProcessorValueTreeState& apvts);
    ~UtilityBarModule();

    void paint(juce::Graphics& g) override;
    void resized() override;

   private:
    juce::AudioProcessorValueTreeState& _apvts;

    CustomComboBox _profilMenu;
    CustomTextButton _resetButton{"reset", ProfilerStyle::Theme::Orange};
    CustomToggleButton _muteSwitch{"Mute"};
    CustomToggleButton _eqSwitch{"EQ"};

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> _muteAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> _eqAttach;

    void resetAllParameters();
};