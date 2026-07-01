#pragma once

#include <JuceHeader.h>

#include "Stylesheet.h"

class ModalOverlay : public juce::Component {
   public:
    ModalOverlay();
    ModalOverlay(std::unique_ptr<juce::Component> content,
                 juce::Rectangle<int> contentBounds);
    ~ModalOverlay() override;

    void setContent(std::unique_ptr<juce::Component> content,
                    juce::Rectangle<int> contentBounds);
    void show();
    void show(std::unique_ptr<juce::Component> content,
              juce::Rectangle<int> contentBounds);
    void dismiss();

    void setDismissOnBackgroundClick(bool shouldDismiss);
    void setOverlayColour(juce::Colour colour);
    void setPanelCornerRadius(float radius);

    std::function<void()> onDismissed;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;
    bool keyPressed(const juce::KeyPress& key) override;

   private:
    std::unique_ptr<juce::Component> _content;
    juce::Rectangle<int> _contentBounds{0, 0, 420, 240};
    bool _dismissOnBackgroundClick = true;
    juce::Colour _overlayColour = juce::Colours::black.withAlpha(0.6f);
    float _panelCornerRadius = 5.0f;

    juce::Rectangle<int> getCentredContentBounds() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModalOverlay)
};
