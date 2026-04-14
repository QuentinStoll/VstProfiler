#include <juceHeader.h>
#include "Components/CustomTickBox.h"
#include "Components/CustomComboBox.h"

class UtilityBarModule : public juce::Component
{
public:
	UtilityBarModule();
	~UtilityBarModule();

	void paint(juce::Graphics& g) override;
	void resized() override;

private:
	CustomComboBox _profilMenu;
	juce::TextButton _resetButton { "reset" };
	CustomTickBox _muteSwitch{ "Mute"};
	CustomTickBox _eqSwitch{ "EQ" };
};