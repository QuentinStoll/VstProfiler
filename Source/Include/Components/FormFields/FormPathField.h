#pragma once

#include <JuceHeader.h>

#include "Components/CustomTextButton.h"
#include "Components/CustomTextEditor.h"
#include "Components/FormFields/FormField.h"

class FormPathField : public FormField {
   public:
    FormPathField(const juce::Identifier& fieldId,
                  const juce::String& labelText,
                  const juce::String& filePatterns = {});
    ~FormPathField() override = default;

    juce::String getPath() const;
    void setPath(const juce::String& path, juce::NotificationType notification = juce::dontSendNotification);
    void setFilePatterns(const juce::String& filePatterns);

    CustomTextEditor& getEditor() noexcept;
    const CustomTextEditor& getEditor() const noexcept;

    CustomTextButton& getBrowseButton() noexcept;
    const CustomTextButton& getBrowseButton() const noexcept;

    int getPreferredHeight() const override;
    juce::var getValue() const override;
    void setValue(const juce::var& value) override;

    std::function<void(const juce::File&)> onFileSelected;

   protected:
    void resizedControl(juce::Rectangle<int> controlArea) override;

   private:
    void browseForPath();

    CustomTextEditor _editor;
    CustomTextButton _browseButton{"...", ProfilerStyle::Theme::Dark};
    std::unique_ptr<juce::FileChooser> _fileChooser;
    juce::String _filePatterns;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FormPathField)
};
