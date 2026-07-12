#include "Views/PlayView.h"

#include "Modules/AdvancedEqModule.h"
#include "Modules/BasicEqModule.h"
#include "ProfileManager.h"
#include "Stylesheet.h"

PlayView::PlayView(ProfilerAudioProcessor& p)
    : _audioProcessor(p),
      _eqDisplay({{"Basic", [&p] { return std::make_unique<BasicEqModule>(p._apvts); }},
                  {"Advanced", [&p] { return std::make_unique<AdvancedEqModule>(p._apvts); }}}),
      _utilityBar(p),
      _masterSliders(p) {
    addAndMakeVisible(_eqDisplay);
    _eqDisplay.changeView(0);

    // Make the utility bar visible
    addAndMakeVisible(_utilityBar);

    // Make the master sliders visible
    addAndMakeVisible(_masterSliders);

    _exportViewport.getVerticalScrollBar().setColour(juce::ScrollBar::thumbColourId, ProfilerStyle::Colors::orange);
    _exportViewport.setViewedComponent(&_exportProfilModule, false);
    _exportViewport.setScrollBarsShown(true, false);
    addChildComponent(_exportViewport);

    _utilityBar.onExportClicked = [this]() {
        showExportProfilModule();
    };

    _utilityBar.onResetCompleted = [this](const juce::String& message) {
        _notificationBanner.clearAction();
        _notificationBanner.showMessage(message,
                                        NotificationBanner::Type::Success,
                                        5000);
        resized();
    };

    _exportProfilModule.onExportClicked = [this](const juce::NamedValueSet& values,
                                                 const juce::File& destinationFile) {
        exportProfil(values, destinationFile);
    };

    _exportProfilModule.onCreateProfileClicked = [this](const juce::NamedValueSet& values) {
        createProfil(values);
    };

    _exportProfilModule.onCancelClicked = [this]() {
        _notificationBanner.clearAction();
        _notificationBanner.dismiss();
        showPlayControls();
    };

    addChildComponent(_notificationBanner);
}

PlayView::~PlayView() {
}

void PlayView::paint(juce::Graphics& g) {
    auto area = getLocalBounds().toFloat();
    const float cornerSize = 5.0f;
    const float strokeThickness = 2.0f; // Thickness of the inner shadow/light cuts

    // 1. Solid cavity background
    g.setColour(ProfilerStyle::Colors::darkerGrey);
    g.fillRoundedRectangle(area, cornerSize);

    juce::Path cavityPath;
    cavityPath.addRoundedRectangle(area, cornerSize);
    g.reduceClipRegion(cavityPath);

    // 2. TOP HORIZONTAL SHADOW
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.drawHorizontalLine(static_cast<int>(area.getY()), area.getX(), area.getRight());
    if (strokeThickness > 1.0f) {
        g.setColour(juce::Colours::black.withAlpha(0.25f));
        g.drawHorizontalLine(static_cast<int>(area.getY() + 1.0f), area.getX(), area.getRight());
    }

    // 3. BOTTOM HORIZONTAL LIGHT
    g.setColour(ProfilerStyle::Colors::lighterGrey.withAlpha(0.4f));
    g.drawHorizontalLine(static_cast<int>(area.getBottom() - 1.0f), area.getX(), area.getRight());

    // 4. OUTER SHARP EDGE
    g.setColour(juce::Colours::black.withAlpha(0.2f));
    g.drawRoundedRectangle(area, cornerSize, 4.0f);
}

void PlayView::resized() {
    auto area = getLocalBounds().reduced(20);

    if (_contentMode == ContentMode::PlayControls) {
        auto areaWidth = area.getWidth();
        auto utilsBarHeight = area.getHeight() * 0.07f;

        auto eqArea = area.removeFromLeft(areaWidth / 2);

        auto utilsBarArea = area.removeFromTop(static_cast<int>(utilsBarHeight));
        _eqDisplay.setBounds(eqArea);

        auto masterArea = area;

        _utilityBar.setBounds(utilsBarArea);

        _masterSliders.setBounds(masterArea);
    } else {
        constexpr auto scrollbarOffset = 15;
        const auto viewportArea = area.withRight(juce::jmin(getLocalBounds().getRight(),
                                                            area.getRight() + scrollbarOffset));
        _exportViewport.setBounds(viewportArea);

        const auto contentWidth = area.getWidth();
        const auto exportHeight = juce::jmax(area.getHeight(),
                                             _exportProfilModule.getRequiredHeight(contentWidth));
        _exportProfilModule.setBounds(0, 0, contentWidth, exportHeight);
    }

    const auto bannerWidth = juce::jmin(_notificationBanner.getIdealWidth(),
                                    juce::jmax(220, getWidth() - 50));

    _notificationBanner.setBounds(getLocalBounds()
                              .withSizeKeepingCentre(bannerWidth, _notificationBanner.getIdealHeight())
                              .withRightX(getWidth() - 25)
                              .withBottomY(getHeight() - 25));
}

void PlayView::showPlayControls() {
    _contentMode = ContentMode::PlayControls;
    _eqDisplay.setVisible(true);
    _utilityBar.setVisible(true);
    _masterSliders.setVisible(true);
    _exportViewport.setVisible(false);
    _exportProfilModule.setVisible(false);
    resized();
    repaint();
}

void PlayView::showExportProfilModule() {
    _notificationBanner.clearAction();
    _notificationBanner.dismiss();

    const auto exportName = getDefaultExportName();
    auto values = _audioProcessor.getProfileManager().getCurrentProfileValues(exportName);
    values.set("irPath", _audioProcessor.isIRLoaded()
                             ? _audioProcessor.getCurrentIRFile().getFullPathName()
                             : juce::String{});
    values.set("ampPath", _audioProcessor.isAmpFileLoaded()
                              ? _audioProcessor.getCurrentAmpFile().getFullPathName()
                              : juce::String{});

    _exportProfilModule.setExportValues(values, getDefaultExportFile(exportName));
    _contentMode = ContentMode::ExportProfil;
    _eqDisplay.setVisible(false);
    _utilityBar.setVisible(false);
    _masterSliders.setVisible(false);
    _exportViewport.setViewPosition(0, 0);
    _exportViewport.setVisible(true);
    _exportProfilModule.setVisible(true);
    resized();
    repaint();
}

void PlayView::exportProfil(const juce::NamedValueSet& values,
                            const juce::File& destinationFile) {
    juce::String errorMessage;
    if (_audioProcessor.getProfileManager().exportProfile(values, destinationFile, &errorMessage)) {
        showPlayControls();
        _notificationBanner.clearAction();
        _notificationBanner.showMessage("Profile exported successfully",
                                        NotificationBanner::Type::Success,
                                        5000);
    } else {
        _notificationBanner.clearAction();
        _notificationBanner.showMessage(errorMessage,
                                        NotificationBanner::Type::Error,
                                        5000);
    }
}

void PlayView::createProfil(const juce::NamedValueSet& values) {
    juce::String errorMessage;
    if (_audioProcessor.getProfileManager().createProfile(values, &errorMessage)) {
        showPlayControls();
        _notificationBanner.clearAction();
        _notificationBanner.showMessage("Profile created successfully",
                                        NotificationBanner::Type::Success,
                                        5000);
    } else {
        _notificationBanner.clearAction();
        _notificationBanner.showMessage(errorMessage,
                                        NotificationBanner::Type::Error,
                                        5000);
    }
}

juce::String PlayView::getDefaultExportName() const {
    const auto& profileManager = _audioProcessor.getProfileManager();

    if (const auto* profile = profileManager.getProfile(profileManager.getCurrentProfileIndex())) {
        if (profile->name.trim().isNotEmpty()) {
            return profile->name;
        }
    }

    return "Exported Profile";
}

juce::File PlayView::getDefaultExportFile(const juce::String& profileName) const {
    auto fileName = juce::File::createLegalFileName(profileName.trim());

    if (fileName.isEmpty()) {
        fileName = "Exported Profile";
    }

    return juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
        .getChildFile(fileName + ProfileManager::profileFileExtension);
}
