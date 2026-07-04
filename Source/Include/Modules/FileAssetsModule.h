#pragma once

#include <JuceHeader.h>

#include "Components/FileStatusCard.h"

class FileAssetsModule : public juce::Component {
   public:
    struct FileSlot {
        FileStatusCard::Options cardOptions;
        juce::String chooserTitle;
        juce::String filePattern;
        std::function<bool(const juce::File&)> loadFile;
        std::function<void()> unloadFile;
        std::function<bool()> isLoaded;
        std::function<juce::File()> getCurrentFile;
    };

    FileAssetsModule(juce::String title,
                     juce::String summary,
                     std::vector<FileSlot> fileSlots);
    ~FileAssetsModule() override = default;

    void refreshFileState();

    void paint(juce::Graphics& g) override;
    void resized() override;

   private:
    juce::String _title;
    juce::String _summary;
    std::vector<FileSlot> _fileSlots;
    std::vector<std::unique_ptr<FileStatusCard>> _cards;
    juce::Label _titleLabel;
    juce::Label _summaryLabel;
    std::unique_ptr<juce::FileChooser> _fileChooser;

    void configureHeaderLabel(juce::Label& label,
                              const juce::String& text,
                              float fontSize,
                              juce::Colour textColour,
                              bool isBold = false);
    void chooseFile(size_t slotIndex);
    void unloadFile(size_t slotIndex);
    int getColumnCount(int availableWidth) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FileAssetsModule)
};
