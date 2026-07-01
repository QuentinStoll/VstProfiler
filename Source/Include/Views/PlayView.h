#include <JuceHeader.h>

#include "Components/CustomTabs.h"
#include "Components/NotificationBanner.h"
#include "Modules/ExportProfilModule.h"
#include "Modules/MasterSlidersModule.h"
#include "Modules/UtilityBarModule.h"
#include "PluginProcessor.h"

class PlayView : public juce::Component {
   public:
    PlayView(ProfilerAudioProcessor& p);
    ~PlayView();

    void paint(juce::Graphics& g) override;
    void resized() override;

   private:
    enum class ContentMode {
        PlayControls,
        ExportProfil
    };

    ProfilerAudioProcessor& _audioProcessor;
    CustomTabs _eqDisplay;

    UtilityBarModule _utilityBar;

    MasterSlidersModule _masterSliders;
    juce::Viewport _exportViewport;
    ExportProfilModule _exportProfilModule;
    NotificationBanner _notificationBanner;
    ContentMode _contentMode = ContentMode::PlayControls;

    void showPlayControls();
    void showExportProfilModule();
    void exportProfil(const juce::NamedValueSet& values, const juce::File& destinationFile);
    void createProfil(const juce::NamedValueSet& values);
    juce::String getDefaultExportName() const;
    juce::File getDefaultExportFile(const juce::String& profileName) const;
};
