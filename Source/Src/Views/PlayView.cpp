#include "Views/PlayView.h"
#include "Styles/Stylesheet.h"

#include "Modules/BasicEqModule.h"
#include "Modules/AdvancedEqModule.h"

PlayView::PlayView(ProfilerAudioProcessor& p)
	: _audioProcessor(p), _masterSliders(p._apvts)
{
	// Initialize the EQ display bar with tabs and set up the callback for tab changes
	addAndMakeVisible(_eqDisplayBar);
	_eqDisplayBar.addTab("Basic", juce::Colours::darkgrey, 0);
	_eqDisplayBar.addTab("Advanced", juce::Colours::darkgrey, 1);
	_eqDisplayBar.onTabChanged = [this](int index) { changeEqModule(index); };
	changeEqModule(0); // Set the initial EQ module to "Basic"

	// Make the utility bar visible
	addAndMakeVisible(_utilityBar);

	// Make the current content (EQ module) visible
	if (_currentContent) {
		addAndMakeVisible(_currentContent.get());
	}
	
	// Make the master sliders visible
	addAndMakeVisible(_masterSliders);
}

PlayView::~PlayView()
{
}

void PlayView::paint(juce::Graphics& g)
{
	auto area = getLocalBounds().toFloat();
	juce::Path path;

	// Draw a rounded rectangle as the background
	path.addRoundedRectangle(area, 5.0f);

	g.setGradientFill(ProfilerStyle::Gradients::vertical(
		area,
		ProfilerStyle::Colors::darkestGrey,
		ProfilerStyle::Colors::darkestGrey.brighter(0.1f),
		0.8f
	));
	g.fillPath(path);
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
	
	_eqDisplayBar.setBounds(tabBarArea.reduced(10));

	_utilityBar.setBounds(utilsBarArea.reduced(10));

	if (_currentContent) {
		_currentContent->setBounds(tabContentArea);
	}
	
	_masterSliders.setBounds(masterArea);
}

void PlayView::changeEqModule(int index)
{
	_currentContent = nullptr;

	switch (index) {
	case 0:
		_currentContent = std::make_unique<BasicEqModule>(_audioProcessor._apvts);
		break;
	case 1:
		_currentContent = std::make_unique<AdvancedEqModule>(_audioProcessor._apvts);
		break;
	default:
		break;
	}

	if (_currentContent) {
		addAndMakeVisible(_currentContent.get());
		resized(); // Update layout to accommodate new content
	}
}