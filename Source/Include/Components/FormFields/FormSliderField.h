#pragma once

#include <JuceHeader.h>

#include "Components/CustomLinearSlider.h"
#include "Components/FormFields/FormField.h"

class FormSliderField : public FormField {
   public:
    FormSliderField(const juce::Identifier& fieldId,
                    const juce::String& labelText,
                    double min,
                    double max,
                    double defaultValue,
                    const juce::String& suffix = {},
                    double step = 0.1);
    ~FormSliderField() override;

    double getSliderValue() const;
    void setSliderValue(double value, juce::NotificationType notification = juce::dontSendNotification);
    void setRange(double min, double max, double step = 0.1);
    void setTextValueSuffix(const juce::String& suffix);
    void setTextBoxVisible(bool shouldBeVisible);

    juce::Slider& getSlider() noexcept;
    const juce::Slider& getSlider() const noexcept;

    int getPreferredHeight() const override;
    juce::var getValue() const override;
    void setValue(const juce::var& value) override;

   protected:
    void resizedControl(juce::Rectangle<int> controlArea) override;

   private:
    CustomLinearSliderLF _customLF;
    juce::Slider _slider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FormSliderField)
};
