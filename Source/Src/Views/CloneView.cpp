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
    juce::Path path;

    path.addRoundedRectangle(area, 5.0f);

    g.setGradientFill(ProfilerStyle::Gradients::vertical(
        area,
        ProfilerStyle::Colors::darkestGrey,
        ProfilerStyle::Colors::darkestGrey.brighter(0.1f),
        0.8f));
    g.fillPath(path);
}

void CloneView::resized() {
    _fileAssetsModule.setBounds(getLocalBounds());
}

void CloneView::changeListenerCallback(juce::ChangeBroadcaster* source) {
    if (source == &_audioProcessor.getProfileManager()) {
        _fileAssetsModule.refreshFileState();
    }
}
