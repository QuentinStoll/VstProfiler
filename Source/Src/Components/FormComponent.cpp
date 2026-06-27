#include "Components/FormComponent.h"

FormTextField& FormComponent::addTextField(const juce::Identifier& fieldId, const juce::String& labelText) {
    auto field = std::make_unique<FormTextField>(fieldId, labelText);
    auto& fieldRef = *field;
    addField(std::move(field));
    return fieldRef;
}

FormComboField& FormComponent::addComboField(const juce::Identifier& fieldId, const juce::String& labelText, const juce::StringArray& options) {
    auto field = std::make_unique<FormComboField>(fieldId, labelText, options);
    auto& fieldRef = *field;
    addField(std::move(field));
    return fieldRef;
}

FormPathField& FormComponent::addPathField(const juce::Identifier& fieldId, const juce::String& labelText, const juce::String& filePatterns) {
    auto field = std::make_unique<FormPathField>(fieldId, labelText, filePatterns);
    auto& fieldRef = *field;
    addField(std::move(field));
    return fieldRef;
}

FormToggleField& FormComponent::addToggleField(const juce::Identifier& fieldId, const juce::String& labelText) {
    auto field = std::make_unique<FormToggleField>(fieldId, labelText);
    auto& fieldRef = *field;
    addField(std::move(field));
    return fieldRef;
}

FormSliderField& FormComponent::addSliderField(const juce::Identifier& fieldId,
                                               const juce::String& labelText,
                                               double min,
                                               double max,
                                               double defaultValue,
                                               const juce::String& suffix,
                                               double step) {
    auto field = std::make_unique<FormSliderField>(fieldId, labelText, min, max, defaultValue, suffix, step);
    auto& fieldRef = *field;
    addField(std::move(field));
    return fieldRef;
}

void FormComponent::clearFields() {
    _fields.clear();
    resized();
}

FormField* FormComponent::findField(const juce::Identifier& fieldId) const {
    for (const auto& field : _fields) {
        if (field->getFieldId() == fieldId) {
            return field.get();
        }
    }

    return nullptr;
}

juce::var FormComponent::getValue(const juce::Identifier& fieldId) const {
    if (const auto* field = findField(fieldId)) {
        return field->getValue();
    }

    return {};
}

void FormComponent::setValue(const juce::Identifier& fieldId, const juce::var& value) {
    if (auto* field = findField(fieldId)) {
        field->setValue(value);
    }
}

juce::NamedValueSet FormComponent::getValues() const {
    juce::NamedValueSet values;

    for (const auto& field : _fields) {
        values.set(field->getFieldId(), field->getValue());
    }

    return values;
}

void FormComponent::setValues(const juce::NamedValueSet& values) {
    for (const auto& field : _fields) {
        if (const auto* value = values.getVarPointer(field->getFieldId())) {
            field->setValue(*value);
        }
    }
}

void FormComponent::setLabelWidth(int width) {
    _labelWidth = juce::jmax(0, width);

    for (const auto& field : _fields) {
        field->setLabelWidth(_labelWidth);
    }

    resized();
}

void FormComponent::setRowGap(int gap) {
    _rowGap = juce::jmax(0, gap);
    resized();
}

int FormComponent::getContentHeight() const {
    int contentHeight = 0;

    for (const auto& field : _fields) {
        if (contentHeight > 0) {
            contentHeight += _rowGap;
        }

        contentHeight += field->getPreferredHeight();
    }

    return contentHeight;
}

void FormComponent::resized() {
    auto area = getLocalBounds();

    for (const auto& field : _fields) {
        const auto rowHeight = juce::jmin(field->getPreferredHeight(), area.getHeight());
        field->setBounds(area.removeFromTop(rowHeight));

        if (area.isEmpty()) {
            return;
        }

        area.removeFromTop(juce::jmin(_rowGap, area.getHeight()));
    }
}

void FormComponent::addField(std::unique_ptr<FormField> field) {
    field->setLabelWidth(_labelWidth);
    field->onValueChanged = [this](FormField& changedField) {
        if (onFieldChanged) {
            onFieldChanged(changedField);
        }

        if (onChange) {
            onChange();
        }
    };

    addAndMakeVisible(*field);
    _fields.push_back(std::move(field));
    resized();
}
