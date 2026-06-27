#include "Components/FormFields/FormTextField.h"

FormTextField::FormTextField(const juce::Identifier& fieldId, const juce::String& labelText)
    : FormField(fieldId, labelText) {
    addAndMakeVisible(_editor);

    _editor.onTextChange = [this]() {
        notifyValueChanged();
    };
}

juce::String FormTextField::getText() const {
    return _editor.getText();
}

void FormTextField::setText(const juce::String& text, juce::NotificationType notification) {
    _editor.setText(text, notification);
}

void FormTextField::setPlaceholderText(const juce::String& text) {
    _editor.setTextToShowWhenEmpty(text, juce::Colours::white.withAlpha(0.35f));
}

CustomTextEditor& FormTextField::getEditor() noexcept {
    return _editor;
}

const CustomTextEditor& FormTextField::getEditor() const noexcept {
    return _editor;
}

juce::var FormTextField::getValue() const {
    return getText();
}

void FormTextField::setValue(const juce::var& value) {
    setText(value.toString(), juce::dontSendNotification);
}

void FormTextField::resizedControl(juce::Rectangle<int> controlArea) {
    _editor.setBounds(controlArea);
}
