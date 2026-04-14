#include <juceHeader.h>
#include "Styles/Stylesheet.h"

#include "Components/CustomTickBox.h"
#include "Components/CustomComboBox.h"
#include "Components/CustomTextButton.h"

class UtilityBarModule : public juce::Component
{
public:
	UtilityBarModule();
	~UtilityBarModule();

	void paint(juce::Graphics& g) override;
	void resized() override;

private:
	CustomComboBox _profilMenu;
	CustomTextButton _resetButton{ "reset", ProfilerStyle::Theme::Orange };
	CustomTickBox _muteSwitch{ "Mute"};
	CustomTickBox _eqSwitch{ "EQ" };
};