#include "Views/PlayView.h"

#include "Modules/AdvancedEqModule.h"
#include "Modules/BasicEqModule.h"
#include "Stylesheet.h"

PlayView::PlayView(ProfilerAudioProcessor& p)
    : _masterSliders(p._apvts),
      _utilityBar(p._apvts),
      _eqDisplay({{"Basic", [&p] { return std::make_unique<BasicEqModule>(p._apvts); }},
                  {"Advanced", [&p] { return std::make_unique<AdvancedEqModule>(p._apvts); }}}) {
    addAndMakeVisible(_eqDisplay);
    _eqDisplay.changeView(0);

    // Make the utility bar visible
    addAndMakeVisible(_utilityBar);

    // Make the master sliders visible
    addAndMakeVisible(_masterSliders);
}

PlayView::~PlayView() {
}

void PlayView::paint(juce::Graphics& g) {
    auto area = getLocalBounds().toFloat();
    juce::Path path;

    // Draw a rounded rectangle as the background
    path.addRoundedRectangle(area, 5.0f);

    g.setGradientFill(ProfilerStyle::Gradients::vertical(
        area,
        ProfilerStyle::Colors::darkestGrey,
        ProfilerStyle::Colors::darkestGrey.brighter(0.1f),
        0.8f));
    g.fillPath(path);
}

void PlayView::resized() {
    auto area = getLocalBounds().reduced(20);
    auto areaWidth = area.getWidth();
    auto utilsBarHeight = area.getHeight() * 0.07f;

    auto eqArea = area.removeFromLeft(areaWidth / 2);

    auto utilsBarArea = area.removeFromTop(static_cast<int>(utilsBarHeight));
    _eqDisplay.setBounds(eqArea);

    auto masterArea = area;

    _utilityBar.setBounds(utilsBarArea);

    _masterSliders.setBounds(masterArea);
}