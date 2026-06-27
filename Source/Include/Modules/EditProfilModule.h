#pragma once

#include <JuceHeader.h>

#include "Components/CustomTextButton.h"
#include "Components/CustomTextEditor.h"
#include "Components/FormComponent.h"

class EditProfilModule : public juce::Component {
   public:
    EditProfilModule();
    ~EditProfilModule() override = default;

    void setProfileNumber(int profileNumber);
    int getProfileNumber() const;

    void paint(juce::Graphics& g) override;
    void resized() override;
    int getRequiredHeight(int width) const;

    std::function<void(int, const juce::NamedValueSet&)> onSaveClicked;
    std::function<void(int)> onDeleteClicked;
    std::function<void()> onBackClicked;

   private:
    void saveProfil();
    void deleteProfil();
    void back();

    int _profileNumber = 0;
    std::unique_ptr<juce::Drawable> _musicIcon;
    CustomTextEditor _profileNameEditor;
    CustomTextButton _saveButton{"Save", ProfilerStyle::Theme::Orange};
    CustomTextButton _deleteButton{"Delete", ProfilerStyle::Theme::Dark};
    CustomTextButton _backButton{"Back", ProfilerStyle::Theme::Dark};
    FormComponent _form;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EditProfilModule)
};
