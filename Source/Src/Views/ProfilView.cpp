#include "Views/ProfilView.h"

#include "Modules/AddProfilModule.h"
#include "Stylesheet.h"

ProfilView::ProfilView(ProfilerAudioProcessor& p)
    : _audioProcessor(p) {
    addAndMakeVisible(_viewport);
    _viewport.getVerticalScrollBar().setColour(juce::ScrollBar::thumbColourId, ProfilerStyle::Colors::orange);
    _viewport.setViewedComponent(&_grid, false);
    _viewport.setScrollBarsShown(true, false);

    _grid.onAddProfileClicked = [this]() {
        showAddProfileModal();
    };

    addChildComponent(_modalOverlay);
    addChildComponent(_notificationBanner);

    auto addProfilModule = std::make_unique<AddProfilModule>();
    addProfilModule->onCreateProfilClicked = [this]() {
        _modalOverlay.dismiss();
        _notificationBanner.clearAction();
        _notificationBanner.showMessage("Profile creation started",
                                        NotificationBanner::Type::Success);
        this->resized();
    };
    addProfilModule->onImportProfilClicked = [this]() {
        _modalOverlay.dismiss();
        _notificationBanner.setAction("Open", []() {
            DBG("Open imported profile action clicked");
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
    constexpr auto scrollbarOffset = 15;
    const auto viewportArea = area.withRight(juce::jmin(getLocalBounds().getRight(), area.getRight() + scrollbarOffset));

    _viewport.setBounds(viewportArea);

    const auto gridWidth = area.getWidth();
    const auto gridHeight = juce::jmax(area.getHeight(), _grid.getRequiredHeight(gridWidth));

    _grid.setBounds(0, 0, gridWidth, gridHeight);
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
