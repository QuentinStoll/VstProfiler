#pragma once

#include <JuceHeader.h>

#include "Components/CustomComboBox.h"
#include "Components/FormFields/FormField.h"

class FormComboField : public FormField {
   public:
    FormComboField(const juce::Identifier& fieldId, const juce::String& labelText, const juce::StringArray& options = {});
    ~FormComboField() override = default;

    void setOptions(const juce::StringArray& options);
    juce::String getSelectedText() const;
    void setSelectedText(const juce::String& text, juce::NotificationType notification = juce::dontSendNotification);

    CustomComboBox& getComboBox() noexcept;
    const CustomComboBox& getComboBox() const noexcept;

    juce::var getValue() const override;
    void setValue(const juce::var& value) override;

   protected:
    void resizedControl(juce::Rectangle<int> controlArea) override;

   private:
    CustomComboBox _comboBox;
    juce::StringArray _options;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FormComboField)
};
