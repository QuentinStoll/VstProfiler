#pragma once

#include <JuceHeader.h>

#include "Components/CustomTextButton.h"
#include "Stylesheet.h"

class FileStatusCard : public juce::Component {
   public:
    enum class Status {
        Empty,
        Loaded,
        Warning,
        Error
    };

    struct Options {
        juce::String title;
        juce::String loadButtonText;
        juce::String unloadButtonText{"Unload"};
        juce::String emptyStatusText{"Not loaded"};
        juce::String loadedStatusText{"Loaded"};
        juce::String warningStatusText{"Warning"};
        juce::String errorStatusText{"Missing"};
        juce::String emptyFileText;
        juce::String emptyPathText;
        ProfilerStyle::Theme loadButtonTheme = ProfilerStyle::Theme::Dark;
        ProfilerStyle::Theme unloadButtonTheme = ProfilerStyle::Theme::Darker;
    };

    explicit FileStatusCard(Options options);
    ~FileStatusCard() override = default;

    void setFileState(bool isLoaded, const juce::File& file);
    void setFileState(Status status, const juce::File& file);

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
    Status _status = Status::Empty;

    void configureLabel(juce::Label& label,
                        const juce::String& text,
                        float fontSize,
                        juce::Colour textColour,
                        juce::Justification justification = juce::Justification::centredLeft,
                        bool isBold = false);
    void updateLabels();
    bool isLoaded() const noexcept;
    bool canUnload() const noexcept;
    juce::String getStatusText() const;
    static juce::Colour getStatusColour(Status status);
    static juce::String getFileNameOrFallback(const juce::File& file,
                                              Status status,
                                              const juce::String& fallback);
    static juce::String getPathOrFallback(const juce::File& file,
                                          Status status,
                                          const juce::String& fallback);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FileStatusCard)
};
