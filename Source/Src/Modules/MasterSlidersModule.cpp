#include "Modules/MasterSlidersModule.h"

#include "PluginProcessor.h"

MasterSlidersModule::MasterSlidersModule(ProfilerAudioProcessor& processor)
    : _audioProcessor(processor) {
    addAndMakeVisible(_masterVolumeKnob);
    addAndMakeVisible(_outputLevelMeter);
    addAndMakeVisible(_gainKnob);
    addAndMakeVisible(_noiseGateKnob);

    auto& apvts = _audioProcessor._apvts;
    _masterVolumeAttachment = std::make_unique<SliderAttachment>(apvts, "master", _masterVolumeKnob.getSlider());
    _gainAttachment = std::make_unique<SliderAttachment>(apvts, "gain", _gainKnob.getSlider());
    _noiseGateAttachment = std::make_unique<SliderAttachment>(apvts, "noise", _noiseGateKnob.getSlider());

    startTimerHz(30);
}

MasterSlidersModule::~MasterSlidersModule() {
    stopTimer();
}

void MasterSlidersModule::paint(juce::Graphics& /*g*/) {}

void MasterSlidersModule::resized() {
    auto area = getLocalBounds();
    auto areaWidth = area.getWidth();

    auto sideArea = area.removeFromLeft(areaWidth / 3);
    auto noiseGateArea = sideArea.removeFromTop(static_cast<int>(getHeight() * 0.5f));
    auto gainArea = sideArea;

    auto masterArea = area.reduced(getWidth() / 16, 0);
    const auto meterMargin = juce::jmax(4, juce::roundToInt(masterArea.getWidth() * 0.06f));
    const auto meterWidth = juce::jmax(8, juce::roundToInt(masterArea.getWidth() * 0.06f));
    const auto meterHeight = juce::jmax(20, juce::roundToInt(masterArea.getHeight() * 0.25f));

    auto meterArea = masterArea.reduced(meterMargin);
    meterArea = meterArea.removeFromRight(meterWidth);
    meterArea = meterArea.removeFromTop(juce::jmin(meterHeight, meterArea.getHeight()));

    _outputLevelMeter.setBounds(meterArea);
    _masterVolumeKnob.setBounds(masterArea);
    _noiseGateKnob.setBounds(noiseGateArea);
    _gainKnob.setBounds(gainArea);
}

void MasterSlidersModule::timerCallback() {
    _outputLevelMeter.setLevel(_audioProcessor.getRmsLevelOutput());
}
