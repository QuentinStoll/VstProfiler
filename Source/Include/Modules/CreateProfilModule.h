#pragma once

#include <JuceHeader.h>

#include "Components/CustomTextButton.h"
#include "Components/CustomTextEditor.h"
#include "Components/FormComponent.h"

class CreateProfilModule : public juce::Component {
   public:
    CreateProfilModule();
    ~CreateProfilModule() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    int getRequiredHeight(int width) const;
    void setProfileName(const juce::String& profileName);
    void resetToDefaults(const juce::String& profileName);

    std::function<void(const juce::NamedValueSet&)> onCreateClicked;
    std::function<void()> onCancelClicked;

   private:
    void createProfil();
    void cancel();

    std::unique_ptr<juce::Drawable> _musicIcon;
    CustomTextEditor _profileNameEditor;
    CustomTextButton _createButton{"Create", ProfilerStyle::Theme::Orange};
    CustomTextButton _cancelButton{"Cancel", ProfilerStyle::Theme::Dark};
    FormComponent _form;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CreateProfilModule)
};
