#include "Modules/CardGridModule.h"

#include "BinaryData.h"
#include "Stylesheet.h"

CardGridModule::CardGridModule() {
    auto imageFile = juce::File::getCurrentWorkingDirectory().getChildFile("Source/Assets/Svg/plus-icon.svg");
    jassert(imageFile.existsAsFile());

    _addProfileImageButton = std::make_unique<CustomImageButton>("Add profile", BinaryData::plusicon_svg, BinaryData::plusicon_svgSize, ProfilerStyle::Theme::Darker);
    _addProfileImageButton->onClick = [this]() {
        if (onAddProfileClicked)
            onAddProfileClicked();
    };
    addAndMakeVisible(*_addProfileImageButton);

    updateProfileButtons();
}

CardGridModule::~CardGridModule() {
}

void CardGridModule::paint(juce::Graphics& /*g*/) {
}

void CardGridModule::setProfileCount(int profileCount) {
    const auto newProfileCount = juce::jmax(0, profileCount);

    if (_profileCount == newProfileCount) {
        return;
    }

    _profileCount = newProfileCount;

    while (_profileNames.size() > _profileCount) {
        _profileNames.remove(_profileNames.size() - 1);
    }

    updateProfileButtons();
    resized();
}

void CardGridModule::setProfileNames(const juce::StringArray& profileNames) {
    _profileNames = profileNames;
    _profileCount = _profileNames.size();
    updateProfileButtons();
    resized();
    repaint();
}

int CardGridModule::getRequiredHeight(int width) const {
    const auto buttonCount = static_cast<int>(_profileButtons.size()) + 1;
    const auto columns = getColumnCountForWidth(width, buttonCount);
    const auto buttonSize = getButtonSizeForWidth(width, columns);

    if (columns <= 0 || buttonCount <= 0) {
        return 0;
    }

    const auto rows = (buttonCount + columns - 1) / columns;
    return rows * buttonSize + (rows - 1) * buttonGap;
}

void CardGridModule::resized() {
    const auto buttonCount = static_cast<int>(_profileButtons.size()) + 1;
    const auto columns = getColumnCount(buttonCount);
    const auto buttonSize = getButtonSize(columns);

    if (columns == 0 || buttonCount == 0) {
        return;
    }

    const auto startX = 0;
    const auto startY = 0;

    for (int index = 0; index < buttonCount; ++index) {
        const auto column = index % columns;
        const auto row = index / columns;
        const auto bounds = juce::Rectangle<int>(
            startX + column * (buttonSize + buttonGap),
            startY + row * (buttonSize + buttonGap),
            buttonSize,
            buttonSize);

        if (index < static_cast<int>(_profileButtons.size())) {
            _profileButtons[static_cast<size_t>(index)]->setBounds(bounds);
        } else {
            _addProfileImageButton->setBounds(bounds);
        }
    }
}

int CardGridModule::getColumnCount(int buttonCount) const {
    return getColumnCountForWidth(getWidth(), buttonCount);
}

int CardGridModule::getColumnCountForWidth(int width, int buttonCount) const {
    const auto columnsThatFit = (width + buttonGap) / (preferredButtonSize + buttonGap);
    const auto targetColumns = juce::jmin(buttonCount, columnsThatFit);

    return juce::jlimit(minColumns, maxColumns, targetColumns);
}

int CardGridModule::getButtonSize(int columns) const {
    return getButtonSizeForWidth(getWidth(), columns);
}

int CardGridModule::getButtonSizeForWidth(int width, int columns) const {
    if (columns <= 0) {
        return 0;
    }

    return juce::jmax(0, (width - (columns - 1) * buttonGap) / columns);
}

void CardGridModule::updateProfileButtons() {
    auto imageFile = juce::File::getCurrentWorkingDirectory().getChildFile("Source/Assets/Svg/music-note.svg");
    jassert(imageFile.existsAsFile());

    while (static_cast<int>(_profileButtons.size()) < _profileCount) {
        const auto profileNumber = static_cast<int>(_profileButtons.size()) + 1;
        auto button = std::make_unique<CustomImageButton>("Profil " + juce::String(profileNumber), BinaryData::musicnote_svg, BinaryData::musicnote_svgSize, ProfilerStyle::Theme::Light);
        button->onClick = [this, profileNumber]() {
            if (onProfileClicked)
                onProfileClicked(profileNumber);
        };
        addAndMakeVisible(*button);
        _profileButtons.push_back(std::move(button));
    }

    while (static_cast<int>(_profileButtons.size()) > _profileCount) {
        removeChildComponent(_profileButtons.back().get());
        _profileButtons.pop_back();
    }

    for (int index = 0; index < static_cast<int>(_profileButtons.size()); ++index) {
        const auto profileName = index < _profileNames.size() && _profileNames[index].isNotEmpty()
                                     ? _profileNames[index]
                                     : "Profil " + juce::String(index + 1);
        _profileButtons[static_cast<size_t>(index)]->setButtonText(profileName);
    }
}
