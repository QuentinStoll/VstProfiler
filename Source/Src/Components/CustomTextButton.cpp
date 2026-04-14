#include "Components/CustomTextButton.h";

//=============================================================================
// CustomTextButtonLF Implementation
//=============================================================================

void CustomTextButtonLF::drawButtonBackground(juce::Graphics& g, juce::Button& button,
    const juce::Colour& backgroundColour,
    bool isMouseOverButton, bool isButtonDown)
{
    auto area = button.getLocalBounds().toFloat();
    auto cornerSize = 4.0f;

    auto baseColour = backgroundColour;
    if (isButtonDown) baseColour = baseColour.darker(0.2f);
    else if (isMouseOverButton) baseColour = baseColour.brighter(0.1f);

    g.setGradientFill(ProfilerStyle::Gradients::vertical(
        area,
        baseColour.brighter(0.2f),
        baseColour.darker(0.2f),
        0.9f
    ));
    g.fillRoundedRectangle(area.reduced(1.0f), cornerSize);
}

void CustomTextButtonLF::drawButtonText(juce::Graphics& g, juce::TextButton& button,
    bool isMouseOverButton, bool isButtonDown)
{
    g.setColour(button.findColour(juce::TextButton::textColourOffId));
    g.setFont(juce::Font(16.0f));
    g.drawText(button.getButtonText(), button.getLocalBounds(),
        juce::Justification::centred, true);
}

//=============================================================================
// CustomTextButton Implementation
//=============================================================================

CustomTextButton::CustomTextButton(const juce::String& buttonName, ProfilerStyle::Theme theme) : juce::TextButton(buttonName)
{
    setTheme(theme);
    setLookAndFeel(&_customLF);
}

CustomTextButton::~CustomTextButton()
{
	setLookAndFeel(nullptr);
}

void CustomTextButton::setTheme(ProfilerStyle::Theme theme)
{
	switch (theme)
	{
    case ProfilerStyle::Theme::Dark:
		setColour(juce::TextButton::buttonColourId, ProfilerStyle::Colors::darkerGrey);
		setColour(juce::TextButton::textColourOffId, ProfilerStyle::Colors::white);
		break;
	case ProfilerStyle::Theme::Light:
		setColour(juce::TextButton::buttonColourId, ProfilerStyle::Colors::darkGrey);
		setColour(juce::TextButton::textColourOffId, ProfilerStyle::Colors::white);
		break;
	case ProfilerStyle::Theme::Orange:
        setColour(juce::TextButton::buttonColourId, ProfilerStyle::Colors::orange);
		setColour(juce::TextButton::textColourOffId, ProfilerStyle::Colors::white);
		break;
	}
}

void CustomTextButton::paint(juce::Graphics& g)
{
	juce::TextButton::paint(g);
}

void CustomTextButton::resized()
{
	juce::TextButton::resized();
}