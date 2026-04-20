#include <JuceHeader.h>

//=============================================================================
// CustomToggleButtonLF Implementation
//=============================================================================

static class CustomToggleButtonLF : public juce::LookAndFeel_V4
{
public:
	void drawTickBox(juce::Graphics& g, juce::Component& button,
		float x, float y, float w, float h,
		bool ticked, bool isEnabled, bool isMouseOver, bool isMouseDown) override;
};

//=============================================================================
// CustomToggleButton Implementation
//=============================================================================

class CustomToggleButton : public juce::ToggleButton
{
public:
	CustomToggleButton(const juce::String& buttonText);
	~CustomToggleButton();

	void paint(juce::Graphics& g) override;
	void resized() override;

private:
	CustomToggleButtonLF _customLF;
};