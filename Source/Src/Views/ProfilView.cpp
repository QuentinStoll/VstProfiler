#include "Views/ProfilView.h"

#include "Modules/AddProfilModule.h"
#include "Stylesheet.h"

ProfilView::ProfilView(ProfilerAudioProcessor& p)
    : _audioProcessor(p) {
    addAndMakeVisible(_viewport);
    _viewport.getVerticalScrollBar().setColour(juce::ScrollBar::thumbColourId, ProfilerStyle::Colors::orange);
    _viewport.setViewedComponent(&_grid, false);
    _viewport.setScrollBarsShown(true, false);
    addChildComponent(_createProfilModule);
    addChildComponent(_editProfilModule);

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

    auto addProfilModule = std::make_unique<AddProfilModule>();
    addProfilModule->onCreateProfilClicked = [this]() {
        _modalOverlay.dismiss();
        _notificationBanner.clearAction();
        _notificationBanner.dismiss();
        showCreateProfilModule();
    };
    addProfilModule->onImportProfilClicked = [this]() {
        _modalOverlay.dismiss();
        _notificationBanner.setAction("Open", [this]() {
            showCreateProfilModule();
        });
        _notificationBanner.showMessage("Profile imported successfully",
                                        NotificationBanner::Type::Success,
                                        5000);
        this->resized();
    };

    _modalOverlay.setContent(std::move(addProfilModule),
                             juce::Rectangle<int>(0, 0, 480, 280));
}

ProfilView::~ProfilView() {
}

void ProfilView::paint(juce::Graphics& g) {
    auto area = getLocalBounds().toFloat();
    juce::Path path;

    path.addRoundedRectangle(area, 5.0f);

    g.setGradientFill(ProfilerStyle::Gradients::vertical(
        area,
        ProfilerStyle::Colors::darkestGrey,
        ProfilerStyle::Colors::darkestGrey.brighter(0.1f),
        0.8f));
    g.fillPath(path);
}

void ProfilView::resized() {
    const auto area = getLocalBounds().reduced(25);

    if (_contentMode == ContentMode::ProfileGrid) {
        constexpr auto scrollbarOffset = 15;
        const auto viewportArea = area.withRight(juce::jmin(getLocalBounds().getRight(), area.getRight() + scrollbarOffset));

        _viewport.setBounds(viewportArea);

        const auto gridWidth = area.getWidth();
        const auto gridHeight = juce::jmax(area.getHeight(), _grid.getRequiredHeight(gridWidth));

        _grid.setBounds(0, 0, gridWidth, gridHeight);
    } else if (_contentMode == ContentMode::CreateProfil) {
        _createProfilModule.setBounds(area);
    } else {
        _editProfilModule.setBounds(area);
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
    _modalOverlay.show();
}

void ProfilView::showCreateProfilModule() {
    _contentMode = ContentMode::CreateProfil;
    _viewport.setVisible(false);
    _createProfilModule.setVisible(true);
    _editProfilModule.setVisible(false);
    resized();
    repaint();
}

void ProfilView::showEditProfilModule(int profileNumber) {
    _editProfilModule.setProfileNumber(profileNumber);
    _contentMode = ContentMode::EditProfil;
    _viewport.setVisible(false);
    _createProfilModule.setVisible(false);
    _editProfilModule.setVisible(true);
    resized();
    repaint();
}
