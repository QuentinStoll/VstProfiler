#pragma once

#include <JuceHeader.h>

#include "Components/CustomTextButton.h"
#include "Stylesheet.h"

class FileStatusCard : public juce::Component {
   public:
    struct Options {
        juce::String title;
        juce::String loadButtonText;
        juce::String unloadButtonText{"Unload"};
        juce::String emptyFileText;
        juce::String emptyPathText;
        ProfilerStyle::Theme loadButtonTheme = ProfilerStyle::Theme::Dark;
        ProfilerStyle::Theme unloadButtonTheme = ProfilerStyle::Theme::Darker;
    };

    explicit FileStatusCard(Options options);
    ~FileStatusCard() override = default;

    void setFileState(bool isLoaded, const juce::File& file);

    std::function<void()> onLoadClicked;
    std::function<void()> onUnloadClicked;

    void paint(juce::Graphics& g) override;
    void resized() override;

   private:
    Options _options;
    CustomTextButton _loadButton;
    CustomTextButton _unloadButton;
    juce::Label _titleLabel;
    juce::Label _statusLabel;
    juce::Label _fileLabel;
    juce::Label _pathLabel;
    juce::File _file;
    bool _isLoaded = false;

    void configureLabel(juce::Label& label,
                        const juce::String& text,
                        float fontSize,
                        juce::Colour textColour,
                        juce::Justification justification = juce::Justification::centredLeft,
                        bool isBold = false);
    void updateLabels();
    static juce::Colour getStatusColour(bool isLoaded);
    static juce::String getFileNameOrFallback(const juce::File& file,
                                              bool isLoaded,
                                              const juce::String& fallback);
    static juce::String getPathOrFallback(const juce::File& file,
                                          bool isLoaded,
                                          const juce::String& fallback);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FileStatusCard)
};
