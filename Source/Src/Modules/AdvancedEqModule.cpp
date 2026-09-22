#include "Modules/AdvancedEqModule.h"

AdvancedEqModule::AdvancedEqModule(juce::AudioProcessorValueTreeState& apvts) {
    for (int band = 0; band < EqBands::count; ++band) {
        const auto index = static_cast<size_t>(band);
        const auto& spec = EqBands::specs[band];

        _gainKnobs[index] = std::make_unique<CustomKnob>(spec.label, EqBands::minDb, EqBands::maxDb, spec.defaultDb, "dB");
        _freqKnobs[index] = std::make_unique<CustomKnob>("Hz", EqBands::minHz, EqBands::maxHz, spec.defaultHz, "Hz", 0.1f);
        addAndMakeVisible(*_gainKnobs[index]);
        addAndMakeVisible(*_freqKnobs[index]);
        _gainAttachments[index] = std::make_unique<SliderAttachment>(apvts, spec.gainId, _gainKnobs[index]->getSlider());
        _freqAttachments[index] = std::make_unique<SliderAttachment>(apvts, spec.freqId, _freqKnobs[index]->getSlider());
    }
}

void AdvancedEqModule::paint(juce::Graphics& /*g*/) {}

void AdvancedEqModule::resized() {
    auto area = getLocalBounds();
    const auto count = EqBands::count;
    const auto gap = 8;
    auto gainRow = area.removeFromTop(area.getHeight() / 2);
    auto freqRow = area;
    const auto knobWidth = juce::jmin(92, juce::jmax(52, (gainRow.getWidth() - gap * (count - 1)) / count));
    const auto totalWidth = count * knobWidth + (count - 1) * gap;
    auto gainBounds = juce::Rectangle<int>(totalWidth, juce::jmin(108, gainRow.getHeight())).withCentre(gainRow.getCentre());
    auto freqBounds = juce::Rectangle<int>(totalWidth, juce::jmin(108, freqRow.getHeight())).withCentre(freqRow.getCentre());

    for (int band = 0; band < count; ++band) {
        _gainKnobs[static_cast<size_t>(band)]->setBounds(gainBounds.removeFromLeft(knobWidth));
        _freqKnobs[static_cast<size_t>(band)]->setBounds(freqBounds.removeFromLeft(knobWidth));
        if (band + 1 < count) {
            gainBounds.removeFromLeft(gap);
            freqBounds.removeFromLeft(gap);
        }
    }
}
