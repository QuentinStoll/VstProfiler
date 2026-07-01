#include "Components/FormFields/FormPathField.h"

FormPathField::FormPathField(const juce::Identifier& fieldId,
                             const juce::String& labelText,
                             const juce::String& filePatterns)
    : FormField(fieldId, labelText),
      _filePatterns(filePatterns) {
    addAndMakeVisible(_editor);
    addAndMakeVisible(_browseButton);

    _editor.onTextChange = [this]() {
        notifyValueChanged();
    };

    _browseButton.onClick = [this]() {
        browseForPath();
    };
}

juce::String FormPathField::getPath() const {
    return _editor.getText();
}

void FormPathField::setPath(const juce::String& path, juce::NotificationType notification) {
    _editor.setText(path, notification);
}

void FormPathField::setFilePatterns(const juce::String& filePatterns) {
    _filePatterns = filePatterns;
}

CustomTextEditor& FormPathField::getEditor() noexcept {
    return _editor;
}

const CustomTextEditor& FormPathField::getEditor() const noexcept {
    return _editor;
}

CustomTextButton& FormPathField::getBrowseButton() noexcept {
    return _browseButton;
}

const CustomTextButton& FormPathField::getBrowseButton() const noexcept {
    return _browseButton;
}

int FormPathField::getPreferredHeight() const {
    return 30;
}

juce::var FormPathField::getValue() const {
    return getPath();
}

void FormPathField::setValue(const juce::var& value) {
    setPath(value.toString(), juce::dontSendNotification);
}

void FormPathField::resizedControl(juce::Rectangle<int> controlArea) {
    const auto browseButtonWidth = 42;

    _browseButton.setBounds(controlArea.removeFromRight(juce::jmin(browseButtonWidth, controlArea.getWidth())));

    if (controlArea.getWidth() > 0) {
        controlArea.removeFromRight(juce::jmin(8, controlArea.getWidth()));
    }

    _editor.setBounds(controlArea);
}

void FormPathField::browseForPath() {
    _fileChooser = std::make_unique<juce::FileChooser>("Select " + getLabelText(), juce::File{}, _filePatterns);
    _fileChooser->launchAsync(juce::FileBrowserComponent::openMode |
                                  juce::FileBrowserComponent::canSelectFiles,
                              [this](const juce::FileChooser& chooser) {
                                  const auto file = chooser.getResult();
                                  if (file.existsAsFile()) {
                                      setPath(file.getFullPathName(), juce::sendNotification);
                                      if (onFileSelected) {
                                          onFileSelected(file);
                                      }
                                  }

                                  _fileChooser.reset();
                              });
}
