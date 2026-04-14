#include <JuceHeader.h>;
#include "Styles/Stylesheet.h"

//=============================================================================
// CustomTextButtonLF Implementation
//=============================================================================

static class CustomTextButtonLF : public juce::LookAndFeel_V4
{
public:
	void drawButtonBackground(juce::Graphics& g, juce::Button& button,
		const juce::Colour& backgroundColour,
		bool isMouseOverButton, bool isButtonDown) override;

	void drawButtonText(juce::Graphics& g, juce::TextButton& button,
		bool isMouseOverButton, bool isButtonDown) override;
};

//=============================================================================
// CustomTextButton Implementation
//=============================================================================

class CustomTextButton : public juce::TextButton
{
public:
	CustomTextButton(const juce::String& buttonName, ProfilerStyle::Theme theme);
	~CustomTextButton() override;

	void setTheme(ProfilerStyle::Theme theme);

	void paint(juce::Graphics& g) override;
	void resized() override;

private:
	CustomTextButtonLF _customLF;
};