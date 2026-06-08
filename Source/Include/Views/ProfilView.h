#include <JuceHeader.h>

#include "Components/ModalOverlay.h"
#include "Modules/CardGridModule.h"
#include "PluginProcessor.h"

class ProfilView : public juce::Component {
   public:
    ProfilView(ProfilerAudioProcessor& p);
    ~ProfilView();

    void paint(juce::Graphics& g) override;
    void resized() override;

   private:
    juce::Viewport _viewport;
    CardGridModule _grid;
    ModalOverlay _modalOverlay;
    ProfilerAudioProcessor& _audioProcessor;

    void showAddProfileModal();
};
