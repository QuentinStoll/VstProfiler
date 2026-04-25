#include <JuceHeader.h>

#include "Components/CustomComboBox.h"
#include "Components/CustomTextButton.h"
#include "Components/CustomToggleButton.h"
#include "Styles/Stylesheet.h"

class UtilityBarModule : public juce::Component {
   public:
    UtilityBarModule();
    ~UtilityBarModule();

    void paint(juce::Graphics& g) override;
    void resized() override;

   private:
    CustomComboBox _profilMenu;
    CustomTextButton _resetButton{"reset", ProfilerStyle::Theme::Orange};
    CustomToggleButton _muteSwitch{"Mute"};
    CustomToggleButton _eqSwitch{"EQ"};
};