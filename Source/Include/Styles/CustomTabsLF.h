#include <JuceHeader.h>
#include "Styles/Stylesheet.h"


class CustomTabsLF : public juce::LookAndFeel_V4
{
public:
	// 
	void drawTabButton(juce::TabBarButton& button, juce::Graphics& g,
		bool isMouseOver, bool isButtonDown) override
	{
		auto area = button.getLocalBounds().toFloat().reduced(2.0f);
		auto buttonColour = button.isFrontTab() ? ProfilerStyle::Colors::darkerGrey : ProfilerStyle::Colors::darkGrey;

		if (isMouseOver && !isButtonDown) buttonColour = buttonColour.brighter(0.1f);

		g.setGradientFill(ProfilerStyle::Gradients::vertical(
			area,
			buttonColour.brighter(0.2f),
			buttonColour.darker(0.2f),
			0.9f
		));

		g.fillRoundedRectangle(area, 2.0f);

		g.setColour(juce::Colours::white);
		g.setFont(14.0f);

		g.drawText(button.getButtonText(), area, juce::Justification::centred);
	}

	// Override to prevent drawing the default tab area background
	void drawTabAreaBehindFrontButton(juce::TabbedButtonBar& bar, juce::Graphics& g,
		int w, int h) override
	{}

	// 
	void drawTabbedButtonBarBackground(juce::TabbedButtonBar& bar, juce::Graphics& g) override
	{
		auto area = bar.getLocalBounds().toFloat();

		// 1. Calculer la largeur totale occupée par tous les onglets
		float totalTabsWidth = 0.0f;
		for (int i = 0; i < bar.getNumTabs(); ++i)
		{
			// On récupère la largeur de chaque bouton d'onglet
			totalTabsWidth += bar.getTabButton(i)->getWidth();
		}

		// 2. Ajouter une petite marge (padding) autour des onglets si tu le souhaites
		float padding = 10.0f;
		float bgWidth = totalTabsWidth + padding;

		// 3. Créer le rectangle de fond centré sur cette largeur calculée
		auto bgArea = area.withSizeKeepingCentre(bgWidth, area.getHeight());

		g.setColour(ProfilerStyle::Colors::darkerGrey);
		g.fillRoundedRectangle(bgArea, 3.0f);
	}

    int getTabButtonBestWidth(juce::TabBarButton& bar, int tabDepth) override
    {
        // All tabs will have the same width and will be 5 times the tab depth (height)
		return tabDepth * 5; 
    }
};