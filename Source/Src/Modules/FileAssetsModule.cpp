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

    addChildComponent(_notificationBanner);
    refreshFileState();
}

void FileAssetsModule::refreshFileState() {
    juce::String issueMessage;
    auto issueType = NotificationBanner::Type::Info;

    for (size_t index = 0; index < _cards.size(); ++index) {
        const auto& slot = _fileSlots[index];
        const auto isLoaded = slot.isLoaded ? slot.isLoaded() : false;
        const auto file = slot.getCurrentFile ? slot.getCurrentFile() : juce::File{};
        const auto status = getStatusForFile(isLoaded, file);

        _cards[index]->setFileState(status, file);

        if (issueMessage.isEmpty() && status == FileStatusCard::Status::Error) {
            issueMessage = slot.cardOptions.title + " file does not exist: " + file.getFullPathName();
            issueType = NotificationBanner::Type::Error;
        } else if (issueMessage.isEmpty() && status == FileStatusCard::Status::Warning) {
            issueMessage = slot.cardOptions.title + " file is referenced but not loaded: " + file.getFullPathName();
            issueType = NotificationBanner::Type::Warning;
        }
    }

    if (issueMessage.isNotEmpty()) {
        showIssue(issueMessage, issueType);
    } else {
        clearIssue();
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

    const auto bannerWidth = juce::jmin(_notificationBanner.getIdealWidth(),
                                        juce::jmax(220, getWidth() - 50));
    _notificationBanner.setBounds(getLocalBounds()
                                      .withSizeKeepingCentre(bannerWidth,
                                                             _notificationBanner.getIdealHeight())
                                      .withRightX(getWidth() - 25)
                                      .withY(25));
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
                                  bool loadedSuccessfully = false;
                                  if (file.existsAsFile() && selectedSlot.loadFile) {
                                      loadedSuccessfully = selectedSlot.loadFile(file);
                                  }

                                  safeThis->refreshFileState();
                                  if (loadedSuccessfully && selectedSlot.successMessage.isNotEmpty()) {
                                      safeThis->_notificationBanner.clearAction();
                                      safeThis->_notificationBanner.showMessage(selectedSlot.successMessage,
                                                                                NotificationBanner::Type::Success,
                                                                                5000);
                                  } else if (!loadedSuccessfully && file.existsAsFile()) {
                                      if (selectedSlot.failureMessage.isNotEmpty()) {
                                          safeThis->showIssue(selectedSlot.failureMessage,
                                                              NotificationBanner::Type::Error);
                                      } else {
                                          safeThis->showIssue("Could not load file: " + file.getFullPathName(),
                                                              NotificationBanner::Type::Error);
                                      }
                                  }
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

void FileAssetsModule::showIssue(const juce::String& message,
                                 NotificationBanner::Type type) {
    if (_currentIssueMessage == message && _notificationBanner.isVisible()) {
        return;
    }

    _currentIssueMessage = message;
    _notificationBanner.clearAction();
    _notificationBanner.showMessage(message, type, 0);
    resized();
}

void FileAssetsModule::clearIssue() {
    _currentIssueMessage.clear();
    _notificationBanner.clearAction();
    _notificationBanner.dismiss();
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

FileStatusCard::Status FileAssetsModule::getStatusForFile(bool isLoaded,
                                                          const juce::File& file) {
    if (isLoaded) {
        return FileStatusCard::Status::Loaded;
    }

    if (file.getFullPathName().trim().isEmpty()) {
        return FileStatusCard::Status::Empty;
    }

    return file.existsAsFile() ? FileStatusCard::Status::Warning
                               : FileStatusCard::Status::Error;
}
