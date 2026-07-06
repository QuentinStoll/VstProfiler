#pragma once

#include <JuceHeader.h>

#include "Components/CustomTextButton.h"

class DeleteProfileConfirmationModule : public juce::Component {
   public:
    explicit DeleteProfileConfirmationModule(const juce::String& profileName);
    ~DeleteProfileConfirmationModule() override = default;

    std::function<void()> onConfirm;
    std::function<void()> onCancel;

    void paint(juce::Graphics& g) override;
    void resized() override;

   private:
    void confirm();
    void cancel();

    juce::String _profileName;
    CustomTextButton _yesButton{"Yes", ProfilerStyle::Theme::Orange};
    CustomTextButton _noButton{"No", ProfilerStyle::Theme::Dark};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeleteProfileConfirmationModule)
};
