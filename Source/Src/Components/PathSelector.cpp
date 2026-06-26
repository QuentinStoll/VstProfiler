#include "Components/PathSelector.h"

#include "Stylesheet.h"

PathSelector::PathSelector(const juce::String& labelText, const juce::String& filePatterns)
    : _filePatterns(filePatterns) {
    addAndMakeVisible(_label);
    addAndMakeVisible(_pathEditor);
    addAndMakeVisible(_browseButton);

    _label.setText(labelText, juce::dontSendNotification);
    _label.setJustificationType(juce::Justification::centredLeft);
    _label.setColour(juce::Label::textColourId, ProfilerStyle::Colors::white);
    _label.setFont(juce::Font(juce::FontOptions().withHeight(14.0f)));

    stylePathEditor();

    _browseButton.onClick = [this]() {
        browseForPath();
    };
}

void PathSelector::paint(juce::Graphics& /*g*/) {
}

void PathSelector::resized() {
    auto area = getLocalBounds();
    const auto controlHeight = juce::jmin(22, area.getHeight());
    auto row = area.withSizeKeepingCentre(area.getWidth(), controlHeight);

    _label.setBounds(row.removeFromLeft(_labelWidth));
    row.removeFromLeft(8);

    const auto buttonWidth = 34;
    _browseButton.setBounds(row.removeFromRight(buttonWidth));
    row.removeFromRight(6);
    _pathEditor.setBounds(row);
}

juce::String PathSelector::getPath() const {
    return _pathEditor.getText();
}

void PathSelector::setPath(const juce::String& path) {
    _pathEditor.setText(path, juce::dontSendNotification);
}

void PathSelector::setLabelWidth(int width) {
    _labelWidth = juce::jmax(0, width);
    resized();
}

void PathSelector::browseForPath() {
    _fileChooser = std::make_unique<juce::FileChooser>("Select " + _label.getText(), juce::File{}, _filePatterns);
    _fileChooser->launchAsync(juce::FileBrowserComponent::openMode |
                                  juce::FileBrowserComponent::canSelectFiles,
                              [this](const juce::FileChooser& chooser) {
                                  const auto file = chooser.getResult();
                                  if (file.existsAsFile()) {
                                      setPath(file.getFullPathName());
                                      if (onFileSelected) {
                                          onFileSelected(file);
                                      }
                                  }

                                  _fileChooser.reset();
                              });
}

void PathSelector::stylePathEditor() {
    _pathEditor.setMultiLine(false);
    _pathEditor.setReturnKeyStartsNewLine(false);
    _pathEditor.setScrollbarsShown(false);
    _pathEditor.setSelectAllWhenFocused(true);
    _pathEditor.setJustification(juce::Justification::centredLeft);
    _pathEditor.setIndents(8, 0);
    _pathEditor.setColour(juce::TextEditor::backgroundColourId, ProfilerStyle::Colors::darkGrey);
    _pathEditor.setColour(juce::TextEditor::textColourId, ProfilerStyle::Colors::white);
    _pathEditor.setColour(juce::TextEditor::outlineColourId, ProfilerStyle::Colors::darkerGrey);
    _pathEditor.setColour(juce::TextEditor::focusedOutlineColourId, ProfilerStyle::Colors::orange);
    _pathEditor.setColour(juce::TextEditor::highlightColourId, ProfilerStyle::Colors::orange.withAlpha(0.35f));
    _pathEditor.setColour(juce::TextEditor::highlightedTextColourId, ProfilerStyle::Colors::white);
}
