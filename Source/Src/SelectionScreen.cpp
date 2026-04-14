#include "SelectionScreen.h"

#include <cstdio>

SelectionScreen::SelectionScreen(ScreenSelectedCallback callback) : _screenSelectedCallback(callback) {
    addAndMakeVisible(_cloneButton);
    addAndMakeVisible(_useButton);

    _cloneButton.onClick = [this] {
        printf("Clone button clicked\n");
        _screenSelectedCallback(ScreenID::Cloning);
    };

    _useButton.onClick = [this] { _screenSelectedCallback(ScreenID::Using); };

    setSize(600, 400);
}

SelectionScreen::~SelectionScreen() {}

void SelectionScreen::paint(juce::Graphics& g) { g.fillAll(juce::Colours::lightgrey); }

void SelectionScreen::resized() {
    auto bounds = getLocalBounds();
    auto buttonWidth = bounds.getWidth() / 2 - 20;
    auto buttonHeight = 40;

    _cloneButton.setBounds(10, bounds.getCentreY() - 20, buttonWidth, buttonHeight);
    _useButton.setBounds(bounds.getCentreX() + 10, bounds.getCentreY() - 20, buttonWidth, buttonHeight);
}