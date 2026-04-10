#include "Views/PlayView.h"
#include "Styles/Stylesheet.h"

PlayView::PlayView(ProfilerAudioProcessor& p)
	: _audioProcessor(p)
{
	_eqDisplayBar.addTab("Basic", juce::Colours::darkgrey, 0);
	_eqDisplayBar.addTab("Advanced", juce::Colours::darkgrey, 1);

	addAndMakeVisible(_tabBarZone);
	addAndMakeVisible(_eqDisplayBar);

	addAndMakeVisible(_utilsBarZone);
	addAndMakeVisible(_utilityBar);

	addAndMakeVisible(_tabContentZone);
	addAndMakeVisible(_basicEqModule);
	
	addAndMakeVisible(_masterZone);
	addAndMakeVisible(_masterSliders);
}

PlayView::~PlayView()
{
	//setLookAndFeel(nullptr);
}

void PlayView::paint(juce::Graphics& g)
{
	auto area = getLocalBounds().toFloat();
	juce::Path path;

	path.addRoundedRectangle(area, 5.0f);

	g.setGradientFill(ProfilerStyle::Gradients::vertical(
		area,
		ProfilerStyle::Colors::darkestGrey,
		ProfilerStyle::Colors::darkestGrey.brighter(0.1f),
		0.8f
	));
	g.fillPath(path);

	// Draw zones with different colors for visualization
	g.setColour(juce::Colours::blue.withAlpha(0.2f));
	g.fillRect(_tabBarZone.getBounds());

	g.setColour(juce::Colours::yellow.withAlpha(0.2f));
	g.fillRect(_utilsBarZone.getBounds());

	g.setColour(juce::Colours::red.withAlpha(0.2f));
	g.fillRect(_tabContentZone.getBounds());

	g.setColour(juce::Colours::green.withAlpha(0.2f));
	g.fillRect(_masterZone.getBounds());
}

void PlayView::resized()
{
	auto area = getLocalBounds().reduced(20);
	auto areaWidth = area.getWidth();

	auto topBarArea = area.removeFromTop(getHeight() * 0.1f);
	auto tabBarArea = topBarArea.removeFromLeft(areaWidth/2);
	auto utilsBarArea = topBarArea;

	area.removeFromTop(10); // Spacing

	auto tabContentArea = area.removeFromLeft(areaWidth/2);
	auto masterArea = area;
	
	_tabBarZone.setBounds(tabBarArea);
	_eqDisplayBar.setBounds(tabBarArea.reduced(10));

	_utilsBarZone.setBounds(utilsBarArea);
	_utilityBar.setBounds(utilsBarArea.reduced(10));

	_tabContentZone.setBounds(tabContentArea);
	_basicEqModule.setBounds(tabContentArea);
	
	_masterZone.setBounds(masterArea);
	_masterSliders.setBounds(masterArea);
}