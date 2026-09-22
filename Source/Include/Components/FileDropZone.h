#pragma once

#include <JuceHeader.h>

#include "Components/CustomLookAndFeel.h"

class FileDropZone : public juce::Component,
                     public juce::FileDragAndDropTarget,
                     public juce::SettableTooltipClient {
   public:
    struct Options {
        juce::String title;
        juce::String emptyDetail;
        juce::String chooserTitle;
        juce::String filePattern;
        CustomLookAndFeel::RigIcon icon = CustomLookAndFeel::RigIcon::AmpHead;
        juce::Colour categoryColour;
        std::function<bool(const juce::File&)> loadFile;
        std::function<void()> unloadFile;
        std::function<bool()> isLoaded;
        std::function<juce::File()> getCurrentFile;
        juce::String successMessage;
        juce::String failureMessage;
    };

    explicit FileDropZone(Options options);
    ~FileDropZone() override = default;

    void refresh();
    bool isFileLoaded() const;

    std::function<void(const juce::String&, bool)> onStatusMessage;
    std::function<void()> onChanged;

    void paint(juce::Graphics& g) override;
    void mouseUp(const juce::MouseEvent& event) override;

    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void fileDragEnter(const juce::StringArray& files, int x, int y) override;
    void fileDragExit(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

   private:
    Options _options;
    bool _isDragOver = false;
    std::unique_ptr<juce::FileChooser> _fileChooser;

    bool matchesPattern(const juce::File& file) const;
    bool tryLoadFile(const juce::File& file);
    void chooseFile();
    void unloadFile();
    juce::String getDetailText() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FileDropZone)
};
