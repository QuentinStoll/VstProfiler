#include "Modules/BasicEqModule.h"

BasicEqModule::BasicEqModule(juce::AudioProcessorValueTreeState& apvts) {
    addAndMakeVisible(_bassKnob);
    addAndMakeVisible(_midKnob);
    addAndMakeVisible(_trebleKnob);
    addAndMakeVisible(_presenceKnob);
    addAndMakeVisible(_depthKnob);

    _bassAttachment = std::make_unique<SliderAttachment>(apvts, "bass", _bassKnob.getSlider());
    _midAttachment = std::make_unique<SliderAttachment>(apvts, "mid", _midKnob.getSlider());
    _trebleAttachment = std::make_unique<SliderAttachment>(apvts, "treble", _trebleKnob.getSlider());
    _presenceAttachment = std::make_unique<SliderAttachment>(apvts, "presence", _presenceKnob.getSlider());
    _depthAttachment = std::make_unique<SliderAttachment>(apvts, "depth", _depthKnob.getSlider());
}

BasicEqModule::~BasicEqModule() {}

void BasicEqModule::paint(juce::Graphics& /*g*/) {}

void BasicEqModule::resized() {
    auto area = getLocalBounds();
    const auto count = 5;
    const auto gap = 12;
    const auto knobWidth = juce::jmin(92, juce::jmax(64, (area.getWidth() - gap * (count - 1)) / count));
    const auto totalWidth = count * knobWidth + (count - 1) * gap;
    auto row = juce::Rectangle<int>(totalWidth, juce::jmin(108, area.getHeight())).withCentre(area.getCentre());

    juce::Component* knobs[] = {&_depthKnob, &_bassKnob, &_midKnob, &_trebleKnob, &_presenceKnob};
    for (int index = 0; index < count; ++index) {
        knobs[index]->setBounds(row.removeFromLeft(knobWidth));
        if (index + 1 < count) {
            row.removeFromLeft(gap);
        }
    }
}