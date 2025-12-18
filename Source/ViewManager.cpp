/*
  ==============================================================================

    ViewManager.cpp
    Created: 30 Nov 2025 4:10:29pm
    Author:  Laurent ZHANG

  ==============================================================================
*/

#include "ViewManager.h"
#include "SelectionScreen.h"
#include "CloningScreen.h"
#include "UsingScreen.h"

ViewManager::ViewManager(ProfilerAudioProcessor& p) : _audioProcessor(p)
{
    _backButton.onClick = [this]() {
        changeView(ScreenID::Selection);
	};
    addAndMakeVisible(_backButton);

    changeView(ScreenID::Selection);

    setSize(600, 400);
}

ViewManager::~ViewManager()
{
}

void ViewManager::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    if (_currentView) {
        _currentView->setBounds(getLocalBounds());
    }
    
    _backButton.setBounds(10, 10, 80, 30);
}

void ViewManager::changeView(ScreenID screenID)
{
    if (_currentView) {
        removeChildComponent(_currentView.get());
		_currentView.reset();
    }

    switch (screenID) {
        case ScreenID::Selection:
            _currentView = std::make_unique<SelectionScreen>([this](ScreenID id) {
                this->changeView(id);
            });
			_backButton.setVisible(false);
			break;
		case ScreenID::Cloning:
            _currentView = std::make_unique<CloningScreen>(_audioProcessor);
			_backButton.setVisible(true);
			break;
        case ScreenID::Using:
			_currentView = std::make_unique<UsingScreen>(_audioProcessor._apvts);
			_backButton.setVisible(true);
            break;
		default:
			break;
    }   

	addAndMakeVisible(_currentView.get());
	_backButton.toFront(true);
    resized();
}