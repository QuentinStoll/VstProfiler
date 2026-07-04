#include <JuceHeader.h>

#include "Components/CustomComboBox.h"
#include "Components/CustomTextButton.h"
#include "Components/CustomToggleButton.h"
#include "Stylesheet.h"

class ProfilerAudioProcessor;

class UtilityBarModule : public juce::Component,
                         private juce::ChangeListener {
   public:
    UtilityBarModule(ProfilerAudioProcessor& processor);
    ~UtilityBarModule();

    void paint(juce::Graphics& g) override;
    void resized() override;

    std::function<void()> onExportClicked;

   private:
    ProfilerAudioProcessor& _audioProcessor;
    juce::AudioProcessorValueTreeState& _apvts;

    CustomComboBox _profilMenu;
    CustomTextButton _resetButton{"Reset", ProfilerStyle::Theme::Orange};
    CustomTextButton _exportButton{"Export", ProfilerStyle::Theme::Dark};
    CustomToggleButton _muteSwitch{"Mute"};
    CustomToggleButton _eqSwitch{"EQ"};

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> _muteAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> _eqAttach;

    bool _isUpdatingProfileMenu = false;

    void resetAllParameters();
    void resetParametersToDefaults();
    void resetLoadedFiles();
    void refreshProfileMenu();
    void restoreLastUsedProfile();
    void selectProfileFromMenu();
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
};
