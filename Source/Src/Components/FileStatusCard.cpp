#include "Components/FileStatusCard.h"

FileStatusCard::FileStatusCard(Options options)
    : _options(std::move(options)),
      _loadButton(_options.loadButtonText, _options.loadButtonTheme),
      _unloadButton(_options.unloadButtonText, _options.unloadButtonTheme) {
    configureLabel(_titleLabel,
                   _options.title,
                   16.0f,
                   ProfilerStyle::Colors::white,
                   juce::Justification::centredLeft,
                   true);
    configureLabel(_statusLabel,
                   {},
                   13.0f,
                   ProfilerStyle::Colors::white.withAlpha(0.58f),
                   juce::Justification::centredRight,
                   true);
    configureLabel(_fileLabel,
                   {},
                   16.0f,
                   ProfilerStyle::Colors::white,
                   juce::Justification::centredLeft,
                   true);
    configureLabel(_pathLabel,
                   {},
                   13.0f,
                   ProfilerStyle::Colors::white.withAlpha(0.62f));

    addAndMakeVisible(_titleLabel);
    addAndMakeVisible(_statusLabel);
    addAndMakeVisible(_fileLabel);
    addAndMakeVisible(_pathLabel);
    addAndMakeVisible(_loadButton);
    addAndMakeVisible(_unloadButton);

    _loadButton.onClick = [this]() {
        if (onLoadClicked) {
            onLoadClicked();
        }
    };
    _unloadButton.onClick = [this]() {
        if (onUnloadClicked) {
            onUnloadClicked();
        }
    };

    updateLabels();
}

void FileStatusCard::setFileState(bool isLoaded, const juce::File& file) {
    _isLoaded = isLoaded;
    _file = file;
    updateLabels();
    resized();
    repaint();
}

void FileStatusCard::paint(juce::Graphics& g) {
    const auto card = getLocalBounds().toFloat();

    g.setGradientFill(ProfilerStyle::Gradients::vertical(
        card,
        ProfilerStyle::Colors::lighterGrey.withAlpha(0.78f),
        ProfilerStyle::Colors::darkerGrey.withAlpha(0.72f),
        0.85f));
    g.fillRoundedRectangle(card, 5.0f);

    g.setColour(getStatusColour(_isLoaded).withAlpha(_isLoaded ? 0.75f : 0.28f));
    g.fillRoundedRectangle(card.withWidth(4.0f), 2.0f);

    g.setColour(ProfilerStyle::Colors::lightestGrey.withAlpha(0.34f));
    g.drawRoundedRectangle(card, 5.0f, 1.0f);
}

void FileStatusCard::resized() {
    auto content = getLocalBounds().reduced(16);
    auto titleRow = content.removeFromTop(24);

    _statusLabel.setBounds(titleRow.removeFromRight(96));
    _titleLabel.setBounds(titleRow);

    content.removeFromTop(14);
    _fileLabel.setBounds(content.removeFromTop(28));

    content.removeFromTop(4);
    _pathLabel.setBounds(content.removeFromTop(24));

    auto buttonArea = content.removeFromBottom(38);
    const auto loadButtonWidth = juce::jmin(148, buttonArea.getWidth());

    if (_isLoaded) {
        constexpr auto gap = 10;
        const auto unloadButtonWidth = juce::jmin(112, juce::jmax(0, buttonArea.getWidth() - loadButtonWidth - gap));
        _loadButton.setBounds(buttonArea.removeFromLeft(loadButtonWidth));
        buttonArea.removeFromLeft(gap);
        _unloadButton.setBounds(buttonArea.removeFromLeft(unloadButtonWidth));
    } else {
        _loadButton.setBounds(buttonArea.withWidth(loadButtonWidth));
        _unloadButton.setBounds({});
    }
}

void FileStatusCard::configureLabel(juce::Label& label,
                                    const juce::String& text,
                                    float fontSize,
                                    juce::Colour textColour,
                                    juce::Justification justification,
                                    bool isBold) {
    auto font = juce::Font(juce::FontOptions(fontSize));
    font.setBold(isBold);

    label.setText(text, juce::dontSendNotification);
    label.setFont(font);
    label.setJustificationType(justification);
    label.setColour(juce::Label::textColourId, textColour);
    label.setMinimumHorizontalScale(0.7f);
    label.setInterceptsMouseClicks(false, false);
}

void FileStatusCard::updateLabels() {
    _statusLabel.setText(_isLoaded ? "Loaded" : "Not loaded",
                         juce::dontSendNotification);
    _statusLabel.setColour(juce::Label::textColourId, getStatusColour(_isLoaded));

    _fileLabel.setText(getFileNameOrFallback(_file, _isLoaded, _options.emptyFileText),
                       juce::dontSendNotification);
    _pathLabel.setText(getPathOrFallback(_file, _isLoaded, _options.emptyPathText),
                       juce::dontSendNotification);

    const auto tooltip = _isLoaded ? _file.getFullPathName() : juce::String{};
    _fileLabel.setTooltip(tooltip);
    _pathLabel.setTooltip(tooltip);

    _unloadButton.setEnabled(_isLoaded);
    _unloadButton.setVisible(_isLoaded);
}

juce::Colour FileStatusCard::getStatusColour(bool isLoaded) {
    return isLoaded ? juce::Colour(0xff38d17a)
                    : ProfilerStyle::Colors::white.withAlpha(0.58f);
}

juce::String FileStatusCard::getFileNameOrFallback(const juce::File& file,
                                                   bool isLoaded,
                                                   const juce::String& fallback) {
    if (!isLoaded) {
        return fallback;
    }

    const auto fileName = file.getFileName();
    return fileName.isNotEmpty() ? fileName : fallback;
}

juce::String FileStatusCard::getPathOrFallback(const juce::File& file,
                                               bool isLoaded,
                                               const juce::String& fallback) {
    if (!isLoaded) {
        return fallback;
    }

    const auto path = file.getFullPathName();
    return path.isNotEmpty() ? path : fallback;
}
