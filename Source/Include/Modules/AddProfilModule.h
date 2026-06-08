#pragma once

#include <JuceHeader.h>

#include "Components/CustomImageButton.h"

class AddProfilModule : public juce::Component {
   public:
    AddProfilModule();
    ~AddProfilModule() override = default;

    std::function<void()> onCreateProfilClicked;
    std::function<void()> onImportProfilClicked;

    void paint(juce::Graphics& g) override;
    void resized() override;

   private:
    void createProfilClicked();
    void importProfilClicked();

    juce::Label _title;
    CustomImageButton _createProfilButton;
    CustomImageButton _importProfilButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AddProfilModule)
};
