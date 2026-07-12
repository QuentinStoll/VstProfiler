#include "Views/ProfilView.h"

#include "Modules/AddProfilModule.h"
#include "Modules/DeleteProfileConfirmationModule.h"
#include "Stylesheet.h"

ProfilView::ProfilView(ProfilerAudioProcessor& p)
    : _audioProcessor(p) {
    addAndMakeVisible(_viewport);
    _viewport.getVerticalScrollBar().setColour(juce::ScrollBar::thumbColourId, ProfilerStyle::Colors::orange);
    _viewport.setViewedComponent(&_grid, false);
    _viewport.setScrollBarsShown(true, false);
    refreshProfileGrid();

    _createProfilModule.onCreateClicked = [this](const juce::NamedValueSet& values) {
        juce::String errorMessage;
        if (_audioProcessor.getProfileManager().createProfile(values, &errorMessage)) {
            refreshProfileGrid();
            _notificationBanner.clearAction();
            _notificationBanner.showMessage("Profile created successfully",
                                            NotificationBanner::Type::Success,
                                            5000);
            showProfileGrid();
        } else {
            _notificationBanner.clearAction();
            _notificationBanner.showMessage(errorMessage,
                                            NotificationBanner::Type::Error,
                                            5000);
        }
    };
    _createProfilModule.onCancelClicked = [this]() {
        showProfileGrid();
    };
    _editProfilModule.onSaveClicked = [this](int profileNumber, const juce::NamedValueSet& values) {
        juce::String errorMessage;
        auto& profileManager = _audioProcessor.getProfileManager();
        const auto profileIndex = profileNumber - 1;
        const auto* profile = profileManager.getProfile(profileIndex);
        const auto isCurrentProfile = profile != nullptr &&
                                      profile->id == profileManager.getCurrentProfileId();

        if (profileManager.updateProfile(profileIndex, values, &errorMessage)) {
            if (isCurrentProfile) {
                _audioProcessor.syncLoadedFilesWithCurrentProfile();
            }

            refreshProfileGrid();
            _notificationBanner.clearAction();
            _notificationBanner.showMessage("Profile saved successfully",
                                            NotificationBanner::Type::Success,
                                            5000);
        } else {
            _notificationBanner.clearAction();
            _notificationBanner.showMessage(errorMessage,
                                            NotificationBanner::Type::Error,
                                            5000);
        }
    };
    _editProfilModule.onDeleteClicked = [this](int profileNumber) {
        showDeleteProfileModal(profileNumber);
    };
    _editProfilModule.onBackClicked = [this]() {
        showProfileGrid();
    };

    _grid.onAddProfileClicked = [this]() {
        showAddProfileModal();
    };

    _grid.onProfileClicked = [this](int profileNumber) {
        _notificationBanner.clearAction();
        _notificationBanner.dismiss();
        showEditProfilModule(profileNumber);
    };

    addChildComponent(_modalOverlay);
    addChildComponent(_notificationBanner);
}

ProfilView::~ProfilView() {
}

void ProfilView::paint(juce::Graphics& g) {
    auto area = getLocalBounds().toFloat();
    juce::Path path;

    path.addRoundedRectangle(area, 5.0f);

    g.setGradientFill(ProfilerStyle::Gradients::vertical(
        area,
        ProfilerStyle::Colors::appBackground,
        ProfilerStyle::Colors::appBackground.brighter(0.1f),
        0.8f));
    g.fillPath(path);
}

void ProfilView::resized() {
    const auto area = getLocalBounds().reduced(25);

    if (_contentMode == ContentMode::ProfileGrid || _contentMode == ContentMode::CreateProfil || _contentMode == ContentMode::EditProfil) {
        constexpr auto scrollbarOffset = 15;
        const auto viewportArea = area.withRight(juce::jmin(getLocalBounds().getRight(), area.getRight() + scrollbarOffset));

        _viewport.setBounds(viewportArea);

        const auto contentWidth = area.getWidth();

        if (_contentMode == ContentMode::ProfileGrid) {
            const auto gridHeight = juce::jmax(area.getHeight(), _grid.getRequiredHeight(contentWidth));
            _grid.setBounds(0, 0, contentWidth, gridHeight);
        } else if (_contentMode == ContentMode::CreateProfil) {
            const auto createHeight = juce::jmax(area.getHeight(), _createProfilModule.getRequiredHeight(contentWidth));
            _createProfilModule.setBounds(0, 0, contentWidth, createHeight);
        } else {
            const auto editHeight = juce::jmax(area.getHeight(), _editProfilModule.getRequiredHeight(contentWidth));
            _editProfilModule.setBounds(0, 0, contentWidth, editHeight);
        }
    }

    _modalOverlay.setBounds(getLocalBounds());

    const auto bannerWidth = juce::jmin(_notificationBanner.getIdealWidth(),
                                        juce::jmax(220, getWidth() - 50));
    _notificationBanner.setBounds(getLocalBounds()
                                      .withSizeKeepingCentre(bannerWidth,
                                                             _notificationBanner.getIdealHeight())
                                      .withRightX(getWidth() - 25)
                                      .withY(25));
}

void ProfilView::showAddProfileModal() {
    auto addProfilModule = std::make_unique<AddProfilModule>();

    addProfilModule->onCreateProfilClicked = [this]() {
        _modalOverlay.dismiss();
        _notificationBanner.clearAction();
        _notificationBanner.dismiss();
        showCreateProfilModule();
    };
    addProfilModule->onImportProfilClicked = [this]() {
        _modalOverlay.dismiss();
        _notificationBanner.clearAction();
        _notificationBanner.dismiss();
        importProfil();
    };

    _modalOverlay.show(std::move(addProfilModule),
                       juce::Rectangle<int>(0, 0, 480, 280));
}

void ProfilView::showDeleteProfileModal(int profileNumber) {
    const auto* profile = _audioProcessor.getProfileManager().getProfile(profileNumber - 1);
    const auto profileName = profile != nullptr ? profile->name : "this profile";
    auto confirmationModule = std::make_unique<DeleteProfileConfirmationModule>(profileName);

    confirmationModule->onConfirm = [this, profileNumber]() {
        _modalOverlay.dismiss();
        deleteProfile(profileNumber);
    };

    confirmationModule->onCancel = [this]() {
        _modalOverlay.dismiss();
    };

    _modalOverlay.show(std::move(confirmationModule),
                       juce::Rectangle<int>(0, 0, 296, 108));
}

void ProfilView::deleteProfile(int profileNumber) {
    juce::String errorMessage;
    auto& profileManager = _audioProcessor.getProfileManager();
    const auto profileIndex = profileNumber - 1;
    const auto* profile = profileManager.getProfile(profileIndex);
    const auto isCurrentProfile = profile != nullptr &&
                                  profile->id == profileManager.getCurrentProfileId();

    if (profileManager.deleteProfile(profileIndex, &errorMessage)) {
        if (isCurrentProfile) {
            _audioProcessor.syncLoadedFilesWithCurrentProfile();
        }

        refreshProfileGrid();
        _notificationBanner.clearAction();
        _notificationBanner.showMessage("Profile deleted successfully",
                                        NotificationBanner::Type::Success,
                                        5000);
        showProfileGrid();
    } else {
        _notificationBanner.clearAction();
        _notificationBanner.showMessage(errorMessage,
                                        NotificationBanner::Type::Error,
                                        5000);
    }
}

void ProfilView::showProfileGrid() {
    refreshProfileGrid();
    _contentMode = ContentMode::ProfileGrid;
    _viewport.setViewedComponent(&_grid, false);
    _viewport.setVisible(true);
    _createProfilModule.setVisible(false);
    _editProfilModule.setVisible(false);
    resized();
    repaint();
}

void ProfilView::showCreateProfilModule() {
    _createProfilModule.resetToDefaults("Profil " + juce::String(_audioProcessor.getProfileManager().getProfileCount() + 1));
    _contentMode = ContentMode::CreateProfil;
    _viewport.setViewedComponent(&_createProfilModule, false);
    _viewport.setViewPosition(0, 0);
    _viewport.setVisible(true);
    _createProfilModule.setVisible(true);
    _editProfilModule.setVisible(false);
    resized();
    repaint();
}

void ProfilView::showEditProfilModule(int profileNumber) {
    const auto* profile = _audioProcessor.getProfileManager().getProfile(profileNumber - 1);
    if (profile == nullptr) {
        _notificationBanner.showMessage("Profile not found",
                                        NotificationBanner::Type::Error,
                                        5000);
        showProfileGrid();
        return;
    }

    _editProfilModule.setProfile(profileNumber, profile->values);
    _contentMode = ContentMode::EditProfil;
    _viewport.setViewedComponent(&_editProfilModule, false);
    _viewport.setViewPosition(0, 0);
    _viewport.setVisible(true);
    _createProfilModule.setVisible(false);
    _editProfilModule.setVisible(true);
    resized();
    repaint();
}

void ProfilView::importProfil() {
    auto& profileManager = _audioProcessor.getProfileManager();
    _profileFileChooser = std::make_unique<juce::FileChooser>(
        "Import a profile",
        profileManager.getProfileDirectory(),
        "*" + juce::String(ProfileManager::profileFileExtension));

    const juce::Component::SafePointer<ProfilView> safeThis(this);
    _profileFileChooser->launchAsync(juce::FileBrowserComponent::openMode |
                                         juce::FileBrowserComponent::canSelectFiles,
                                     [safeThis](const juce::FileChooser& chooser) {
                                         if (safeThis == nullptr) {
                                             return;
                                         }

                                         const auto file = chooser.getResult();
                                         if (file.existsAsFile()) {
                                             juce::String errorMessage;
                                             if (safeThis->_audioProcessor.getProfileManager().importProfile(file, &errorMessage)) {
                                                 safeThis->refreshProfileGrid();
                                                 safeThis->_notificationBanner.clearAction();
                                                 safeThis->_notificationBanner.showMessage("Profile imported successfully",
                                                                                           NotificationBanner::Type::Success,
                                                                                           5000);
                                             } else {
                                                 safeThis->_notificationBanner.clearAction();
                                                 safeThis->_notificationBanner.showMessage(errorMessage,
                                                                                           NotificationBanner::Type::Error,
                                                                                           5000);
                                             }
                                         }

                                         safeThis->_profileFileChooser.reset();
                                     });
}

void ProfilView::refreshProfileGrid() {
    auto& profileManager = _audioProcessor.getProfileManager();
    profileManager.refreshProfiles();
    _grid.setProfileNames(profileManager.getProfileNames());
    resized();
}
