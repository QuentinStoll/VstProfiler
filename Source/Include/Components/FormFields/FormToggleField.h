#pragma once

#include <JuceHeader.h>

#include "Components/CustomToggleButton.h"
#include "Components/FormFields/FormField.h"

class FormToggleField : public FormField {
   public:
    FormToggleField(const juce::Identifier& fieldId, const juce::String& labelText);
    ~FormToggleField() override = default;

    bool getToggleState() const;
    void setToggleState(bool shouldBeOn, juce::NotificationType notification = juce::dontSendNotification);

    CustomToggleButton& getToggleButton() noexcept;
    const CustomToggleButton& getToggleButton() const noexcept;

    juce::var getValue() const override;
    void setValue(const juce::var& value) override;

   protected:
    void resizedControl(juce::Rectangle<int> controlArea) override;

   private:
    CustomToggleButton _toggleButton{""};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FormToggleField)
};
