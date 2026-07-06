#pragma once

#include <JuceHeader.h>

#include "Stylesheet.h"

class NotificationBanner : public juce::Component,
                           private juce::Timer {
   public:
    enum class Type {
        Info,
        Success,
        Warning,
        Error
    };

    NotificationBanner();
    ~NotificationBanner() override;

    void setMessage(const juce::String& message);
    void showMessage(const juce::String& message,
                     Type type = Type::Info,
                     int timeoutMs = defaultTimeoutMs);
    void dismiss();

    void setType(Type type);
    void setAction(const juce::String& label, std::function<void()> action);
    void clearAction();

    int getIdealWidth() const;
    int getIdealHeight() const;

    std::function<void()> onDismissed;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseMove(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

   private:
    static constexpr int defaultTimeoutMs = 3500;
    static constexpr int bannerHeight = 42;

    juce::String _message;
    Type _type = Type::Info;
    juce::String _actionLabel;
    std::function<void()> _action;
    std::unique_ptr<juce::Drawable> _dismissIcon;

    juce::Rectangle<int> _messageBounds;
    juce::Rectangle<int> _actionBounds;
    juce::Rectangle<int> _dismissBounds;

    void timerCallback() override;
    void updateMouseCursor(juce::Point<int> position);
    juce::Colour getAccentColour() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NotificationBanner)
};
