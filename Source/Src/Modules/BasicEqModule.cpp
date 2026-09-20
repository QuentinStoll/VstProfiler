#include "Modules/BasicEqModule.h"

BasicEqModule::BasicEqModule(juce::AudioProcessorValueTreeState& apvts) {
    for (int band = 0; band < EqBands::count; ++band) {
        const auto index = static_cast<size_t>(band);
        const auto& spec = EqBands::specs[band];
        _knobs[index] = std::make_unique<CustomKnob>(spec.label, EqBands::minDb, EqBands::maxDb, 0.0f, "dB");
        addAndMakeVisible(*_knobs[index]);
        _attachments[index] = std::make_unique<SliderAttachment>(apvts, spec.gainId, _knobs[index]->getSlider());
    }
}

void BasicEqModule::paint(juce::Graphics& /*g*/) {}

void BasicEqModule::resized() {
    auto area = getLocalBounds();
    const auto count = EqBands::count;
    const auto gap = 8;
    const auto knobWidth = juce::jmin(84, juce::jmax(52, (area.getWidth() - gap * (count - 1)) / count));
    const auto totalWidth = count * knobWidth + (count - 1) * gap;
    auto row = juce::Rectangle<int>(totalWidth, juce::jmin(108, area.getHeight())).withCentre(area.getCentre());

    for (int band = 0; band < count; ++band) {
        _knobs[static_cast<size_t>(band)]->setBounds(row.removeFromLeft(knobWidth));
        if (band + 1 < count) {
            row.removeFromLeft(gap);
        }
    }
}
