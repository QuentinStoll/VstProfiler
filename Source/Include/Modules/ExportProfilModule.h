#pragma once

#include <JuceHeader.h>

#include "Components/CustomTextButton.h"
#include "Components/CustomTextEditor.h"
#include "Components/FormComponent.h"

class ExportProfilModule : public juce::Component {
   public:
    ExportProfilModule();
    ~ExportProfilModule() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    int getRequiredHeight(int width) const;

    void setExportValues(const juce::NamedValueSet& values,
                         const juce::File& defaultExportFile);

    std::function<void(const juce::NamedValueSet&, const juce::File&)> onExportClicked;
    std::function<void(const juce::NamedValueSet&)> onCreateProfileClicked;
    std::function<void()> onCancelClicked;

   private:
    void exportProfil();
    void createProfil();
    void cancel();
    juce::NamedValueSet getEditedValues() const;
    juce::File getDefaultExportFileForName(const juce::String& profileName) const;

    std::unique_ptr<juce::Drawable> _musicIcon;
    CustomTextEditor _profileNameEditor;
    CustomTextButton _exportButton{"Export", ProfilerStyle::Theme::Orange};
    CustomTextButton _createButton{"Create", ProfilerStyle::Theme::Dark};
    CustomTextButton _cancelButton{"Cancel", ProfilerStyle::Theme::Dark};
    FormComponent _form;
    std::unique_ptr<juce::FileChooser> _fileChooser;
    juce::File _defaultExportDirectory;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExportProfilModule)
};
