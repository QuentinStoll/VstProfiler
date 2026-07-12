#include "Views/CloneView.h"

#include "PluginProcessor.h"
#include "Stylesheet.h"

namespace {
std::vector<FileAssetsModule::FileSlot> createCloneFileSlots(ProfilerAudioProcessor& processor) {
    FileAssetsModule::FileSlot irSlot;
    irSlot.cardOptions.title = "IR";
    irSlot.cardOptions.loadButtonText = "Load IR";
    irSlot.cardOptions.emptyFileText = "No IR file loaded";
    irSlot.cardOptions.emptyPathText = "No IR path";
    irSlot.cardOptions.loadButtonTheme = ProfilerStyle::Theme::Orange;
    irSlot.chooserTitle = "Select an IR file";
    irSlot.filePattern = "*.wav;*.aiff;*.aif;*.flac";
    irSlot.loadFile = [&processor](const juce::File& file) {
        return processor.loadIRFile(file);
    };
    irSlot.unloadFile = [&processor]() {
        processor.unloadIRFile();
    };
    irSlot.isLoaded = [&processor]() {
        return processor.isIRLoaded();
    };
    irSlot.getCurrentFile = [&processor]() {
        return processor.getCurrentIRFile();
    };

    FileAssetsModule::FileSlot ampSlot;
    ampSlot.cardOptions.title = "Amp";
    ampSlot.cardOptions.loadButtonText = "Load Amp";
    ampSlot.cardOptions.emptyFileText = "No amp file loaded";
    ampSlot.cardOptions.emptyPathText = "No amp path";
    ampSlot.chooserTitle = "Select an Amp file";
    ampSlot.filePattern = "*.nam;*.json;*.txt";
    ampSlot.loadFile = [&processor](const juce::File& file) {
        return processor.loadAmpFile(file);
    };
    ampSlot.unloadFile = [&processor]() {
        processor.unloadAmpFile();
    };
    ampSlot.isLoaded = [&processor]() {
        return processor.isAmpFileLoaded();
    };
    ampSlot.getCurrentFile = [&processor]() {
        return processor.getCurrentAmpFile();
    };
    std::vector<FileAssetsModule::FileSlot> slots;
    slots.push_back(std::move(irSlot));
    slots.push_back(std::move(ampSlot));
    return slots;
}
}  // namespace

CloneView::CloneView(ProfilerAudioProcessor& p)
    : _audioProcessor(p),
      _fileAssetsModule("Clone Assets",
                        "External clone files currently selected for the plugin.",
                        createCloneFileSlots(p)) {
    addAndMakeVisible(_fileAssetsModule);
    _audioProcessor.getProfileManager().addChangeListener(this);
}

CloneView::~CloneView() {
    _audioProcessor.getProfileManager().removeChangeListener(this);
    setLookAndFeel(nullptr);
}

void CloneView::paint(juce::Graphics& g) {
    auto area = getLocalBounds().toFloat();
    const float cornerSize = 5.0f;
    const float thickness = 8.0f; // Thickness of the shadow borders

    // 1. Solid cavity background
    g.setColour(ProfilerStyle::Colors::darkerGrey);
    g.fillRoundedRectangle(area, cornerSize);

    // Clip to the rounded rectangle so the edges follow the corners perfectly
    juce::Path cavityPath;
    cavityPath.addRoundedRectangle(area, cornerSize);
    g.reduceClipRegion(cavityPath);

    // 2. TOP BORDER (Vertical Gradient)
    // Fades from dark black at the top edge to transparent at the bottom of the thickness
    juce::ColourGradient topGradient(
        juce::Colours::black.withAlpha(0.5f), 0.0f, area.getY(),
        juce::Colours::transparentBlack, 0.0f, area.getY() + thickness, false);
    
    g.setGradientFill(topGradient);
    g.fillRect(area.getX(), area.getY(), area.getWidth(), thickness);

    // 3. LEFT BORDER (Horizontal Gradient)
    // Fades from dark black at the left edge to transparent at the right of the thickness
    juce::ColourGradient leftGradient(
        juce::Colours::black.withAlpha(0.5f), area.getX(), 0.0f,
        juce::Colours::transparentBlack, area.getX() + thickness, 0.0f, false);
    
    g.setGradientFill(leftGradient);
    g.fillRect(area.getX(), area.getY(), thickness, area.getHeight());
}

void CloneView::resized() {
    _fileAssetsModule.setBounds(getLocalBounds());
}

void CloneView::changeListenerCallback(juce::ChangeBroadcaster* source) {
    if (source == &_audioProcessor.getProfileManager()) {
        _fileAssetsModule.refreshFileState();
    }
}
