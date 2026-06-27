#pragma once

#include <JuceHeader.h>

#include "Components/CustomTextEditor.h"
#include "Components/FormFields/FormField.h"

class FormTextField : public FormField {
   public:
    FormTextField(const juce::Identifier& fieldId, const juce::String& labelText);
    ~FormTextField() override = default;

    juce::String getText() const;
    void setText(const juce::String& text, juce::NotificationType notification = juce::dontSendNotification);
    void setPlaceholderText(const juce::String& text);

    CustomTextEditor& getEditor() noexcept;
    const CustomTextEditor& getEditor() const noexcept;

    juce::var getValue() const override;
    void setValue(const juce::var& value) override;

   protected:
    void resizedControl(juce::Rectangle<int> controlArea) override;

   private:
    CustomTextEditor _editor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FormTextField)
};
