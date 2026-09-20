#pragma once

#include <JuceHeader.h>

//==============================================================================
// CustomKnob Implementation
//==============================================================================
class CustomKnob : public juce::Component {
   public:
    CustomKnob(const juce::String& name, float min, float max, float defaultValue, const juce::String& suffix, float step = 0.1f);
    ~CustomKnob() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    juce::Slider& getSlider() { return _slider; };

   private:
    class FineSlider : public juce::Slider {
       public:
        void mouseDown(const juce::MouseEvent& event) override;
        void mouseDrag(const juce::MouseEvent& event) override;
        void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;

       private:
        void applyDragSensitivity(const juce::MouseEvent& event);
    };

    FineSlider _slider;
    juce::Label _label;
};
