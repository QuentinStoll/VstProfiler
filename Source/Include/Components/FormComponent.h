#pragma once

#include <JuceHeader.h>

#include "Components/FormFields/FormComboField.h"
#include "Components/FormFields/FormField.h"
#include "Components/FormFields/FormPathField.h"
#include "Components/FormFields/FormSliderField.h"
#include "Components/FormFields/FormTextField.h"
#include "Components/FormFields/FormToggleField.h"

class FormComponent : public juce::Component {
   public:
    FormComponent() = default;
    ~FormComponent() override = default;

    FormTextField& addTextField(const juce::Identifier& fieldId, const juce::String& labelText);
    FormComboField& addComboField(const juce::Identifier& fieldId, const juce::String& labelText, const juce::StringArray& options = {});
    FormPathField& addPathField(const juce::Identifier& fieldId, const juce::String& labelText, const juce::String& filePatterns = {});
    FormToggleField& addToggleField(const juce::Identifier& fieldId, const juce::String& labelText);
    FormSliderField& addSliderField(const juce::Identifier& fieldId,
                                    const juce::String& labelText,
                                    double min,
                                    double max,
                                    double defaultValue,
                                    const juce::String& suffix = {},
                                    double step = 0.1);

    void clearFields();
    FormField* findField(const juce::Identifier& fieldId) const;

    juce::var getValue(const juce::Identifier& fieldId) const;
    void setValue(const juce::Identifier& fieldId, const juce::var& value);
    juce::NamedValueSet getValues() const;
    void setValues(const juce::NamedValueSet& values);

    void setLabelWidth(int width);
    void setRowGap(int gap);
    int getContentHeight() const;

    void resized() override;

    std::function<void(const FormField&)> onFieldChanged;
    std::function<void()> onChange;

   private:
    void addField(std::unique_ptr<FormField> field);

    std::vector<std::unique_ptr<FormField>> _fields;
    int _labelWidth = 132;
    int _rowGap = 10;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FormComponent)
};
