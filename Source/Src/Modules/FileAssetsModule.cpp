#include "Modules/FileAssetsModule.h"

#include "Stylesheet.h"

FileAssetsModule::FileAssetsModule(juce::String title,
                                   juce::String summary,
                                   std::vector<FileSlot> fileSlots)
    : _title(std::move(title)),
      _summary(std::move(summary)),
      _fileSlots(std::move(fileSlots)) {
    configureHeaderLabel(_titleLabel,
                         _title,
                         24.0f,
                         ProfilerStyle::Colors::white,
                         true);
    configureHeaderLabel(_summaryLabel,
                         _summary,
                         14.0f,
                         ProfilerStyle::Colors::white.withAlpha(0.72f));

    addAndMakeVisible(_titleLabel);
    addAndMakeVisible(_summaryLabel);

    for (size_t index = 0; index < _fileSlots.size(); ++index) {
        auto card = std::make_unique<FileStatusCard>(_fileSlots[index].cardOptions);
        card->onLoadClicked = [this, index]() {
            chooseFile(index);
        };
        card->onUnloadClicked = [this, index]() {
            unloadFile(index);
        };

        addAndMakeVisible(*card);
        _cards.push_back(std::move(card));
    }

    refreshFileState();
}

void FileAssetsModule::refreshFileState() {
    for (size_t index = 0; index < _cards.size(); ++index) {
        const auto& slot = _fileSlots[index];
        const auto isLoaded = slot.isLoaded ? slot.isLoaded() : false;
        const auto file = slot.getCurrentFile ? slot.getCurrentFile() : juce::File{};
        _cards[index]->setFileState(isLoaded, file);
    }
}

void FileAssetsModule::paint(juce::Graphics& /*g*/) {
}

void FileAssetsModule::resized() {
    auto area = getLocalBounds().reduced(24);

    auto headerArea = area.removeFromTop(58);
    _titleLabel.setBounds(headerArea.removeFromTop(30));
    _summaryLabel.setBounds(headerArea.removeFromTop(24));

    area.removeFromTop(10);

    const auto cardCount = static_cast<int>(_cards.size());
    if (cardCount == 0) {
        return;
    }

    constexpr auto gap = 14;
    const auto columns = getColumnCount(area.getWidth());
    const auto rows = (cardCount + columns - 1) / columns;
    const auto cardWidth = (area.getWidth() - (columns - 1) * gap) / columns;
    const auto cardHeight = (area.getHeight() - (rows - 1) * gap) / rows;

    for (int index = 0; index < cardCount; ++index) {
        const auto column = index % columns;
        const auto row = index / columns;
        const auto x = area.getX() + column * (cardWidth + gap);
        const auto y = area.getY() + row * (cardHeight + gap);

        _cards[static_cast<size_t>(index)]->setBounds(x, y, cardWidth, cardHeight);
    }
}

void FileAssetsModule::configureHeaderLabel(juce::Label& label,
                                            const juce::String& text,
                                            float fontSize,
                                            juce::Colour textColour,
                                            bool isBold) {
    auto font = juce::Font(juce::FontOptions(fontSize));
    font.setBold(isBold);

    label.setText(text, juce::dontSendNotification);
    label.setFont(font);
    label.setJustificationType(juce::Justification::centredLeft);
    label.setColour(juce::Label::textColourId, textColour);
    label.setMinimumHorizontalScale(0.7f);
    label.setInterceptsMouseClicks(false, false);
}

void FileAssetsModule::chooseFile(size_t slotIndex) {
    if (slotIndex >= _fileSlots.size()) {
        return;
    }

    const auto& slot = _fileSlots[slotIndex];
    _fileChooser = std::make_unique<juce::FileChooser>(
        slot.chooserTitle,
        juce::File{},
        slot.filePattern);

    const juce::Component::SafePointer<FileAssetsModule> safeThis(this);
    _fileChooser->launchAsync(juce::FileBrowserComponent::openMode |
                                  juce::FileBrowserComponent::canSelectFiles,
                              [safeThis, slotIndex](const juce::FileChooser& chooser) {
                                  if (safeThis == nullptr || slotIndex >= safeThis->_fileSlots.size()) {
                                      return;
                                  }

                                  const auto file = chooser.getResult();
                                  const auto& selectedSlot = safeThis->_fileSlots[slotIndex];
                                  if (file.existsAsFile() && selectedSlot.loadFile) {
                                      selectedSlot.loadFile(file);
                                  }

                                  safeThis->refreshFileState();
                                  safeThis->_fileChooser.reset();
                              });
}

void FileAssetsModule::unloadFile(size_t slotIndex) {
    if (slotIndex >= _fileSlots.size()) {
        return;
    }

    if (_fileSlots[slotIndex].unloadFile) {
        _fileSlots[slotIndex].unloadFile();
    }

    refreshFileState();
}

int FileAssetsModule::getColumnCount(int availableWidth) const {
    const auto cardCount = static_cast<int>(_cards.size());
    if (cardCount <= 1 || availableWidth < 520) {
        return 1;
    }

    if (availableWidth < 760) {
        return juce::jmin(cardCount, 2);
    }

    return juce::jmin(cardCount, 3);
}
