#include "JuceHeader.h"
#include "PluginProcessor.h"

#include "Styles/CustomButtonLF.h"
#include "Components/CustomSlider.h"
#include "Components/CustomTabBar.h"

#include "Modules/BasicEqModule.h"
#include "Modules/MasterSlidersModule.h"
#include "Modules/UtilityBarModule.h"


class PlayView : public juce::Component
{
public:
	PlayView(ProfilerAudioProcessor& p);
	~PlayView();

	void paint(juce::Graphics& g) override;
	void resized() override;

private:
	ProfilerAudioProcessor& _audioProcessor;

	juce::Component _tabBarZone;
	CustomTabBar _eqDisplayBar;

	juce::Component _utilsBarZone;
	UtilityBarModule _utilityBar;
	
	juce::Component _tabContentZone;
	BasicEqModule _basicEqModule;

	juce::Component _masterZone;
	MasterSlidersModule _masterSliders;
};