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

ViewManager::ViewManager()
{
    backButton.onClick = [this]() {
        changeView(ScreenID::Selection);
	};
    addAndMakeVisible(backButton);

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
    if (currentView) {
        currentView->setBounds(getLocalBounds());
    }
    
    backButton.setBounds(10, 10, 80, 30);
}

void ViewManager::changeView(ScreenID screenID)
{
    if (currentView) {
        removeChildComponent(currentView.get());
		currentView.reset();
    }

    switch (screenID) {
    case ScreenID::Selection:
            currentView = std::make_unique<SelectionScreen>([this](ScreenID id) {
                this->changeView(id);
            });
			backButton.setVisible(false);
			break;
		case ScreenID::Cloning:
            currentView = std::make_unique<CloningScreen>();
			backButton.setVisible(true);
			break;
        case ScreenID::Using:
			currentView = std::make_unique<UsingScreen>();
			backButton.setVisible(true);
            break;
		default:
			break;
    }   

	addAndMakeVisible(currentView.get());
	backButton.toFront(true);
    resized();
}