#pragma once
#include <JuceHeader.h>

#include <functional>

#include "ScreenID.h"

class SelectionScreen : public juce::Component {
   public:
    using ScreenSelectedCallback = std::function<void(ScreenID)>;

    SelectionScreen(ScreenSelectedCallback callback);
    ~SelectionScreen();

    void paint(juce::Graphics& g) override;
    void resized() override;

   private:
    juce::TextButton _cloneButton{"Clone"};
    juce::TextButton _useButton{"Use"};

    ScreenSelectedCallback _screenSelectedCallback;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SelectionScreen)
};