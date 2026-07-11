#pragma once
#include <JuceHeader.h>

namespace Gui {
class VerticalLevelMeter : public juce::Component {
   public:
    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        g.setColour(juce::Colours::white.withBrightness(0.4f));
        g.fillRoundedRectangle(bounds, 5.0f);

        g.setColour(juce::Colours::white);
        const auto meterHeight = juce::jmap(juce::jlimit(-60.0f, 6.0f, level),
                                            -60.0f, 6.0f, 0.0f,
                                            static_cast<float>(getHeight()));
        g.fillRoundedRectangle(0.0f,
                               static_cast<float>(getHeight()) - meterHeight,
                               static_cast<float>(getWidth()),
                               meterHeight,
                               5.0f);
    }

    void setLevel(float value) {
        level = juce::jlimit(-60.0f, 6.0f, value);
        repaint();
    }

   private:
    float level = -60.f;
};
}  // namespace Gui
