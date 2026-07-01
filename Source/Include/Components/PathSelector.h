#pragma once

#include <JuceHeader.h>

#include "Components/CustomTextButton.h"
#include "Components/CustomTextEditor.h"

class PathSelector : public juce::Component {
   public:
    PathSelector(const juce::String& labelText, const juce::String& filePatterns = {});
    ~PathSelector() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    juce::String getPath() const;
    void setPath(const juce::String& path);
    void setLabelWidth(int width);

    std::function<void(const juce::File&)> onFileSelected;

   private:
    void browseForPath();

    juce::Label _label;
    CustomTextEditor _pathEditor;
    CustomTextButton _browseButton{"...", ProfilerStyle::Theme::Dark};
    std::unique_ptr<juce::FileChooser> _fileChooser;
    juce::String _filePatterns;
    int _labelWidth = 132;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PathSelector)
};
