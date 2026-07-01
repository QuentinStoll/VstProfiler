#include "Modules/DeleteProfileConfirmationModule.h"

#include "Stylesheet.h"

DeleteProfileConfirmationModule::DeleteProfileConfirmationModule(const juce::String& profileName)
    : _profileName(profileName) {
    addAndMakeVisible(_yesButton);
    addAndMakeVisible(_noButton);

    _yesButton.onClick = [this]() {
        confirm();
    };

    _noButton.onClick = [this]() {
        cancel();
    };
}

void DeleteProfileConfirmationModule::paint(juce::Graphics& g) {
    auto textArea = getLocalBounds().withTrimmedTop(16).removeFromTop(42);

    g.setColour(ProfilerStyle::Colors::white);
    g.setFont(juce::Font(juce::FontOptions(16.0f)).boldened());

    const auto message = juce::String("Are you sure you want to\ndelete ") + _profileName + " ?";
    g.drawFittedText(message,
                     textArea,
                     juce::Justification::centred,
                     2);
}

void DeleteProfileConfirmationModule::resized() {
    auto buttonRow = getLocalBounds().withTrimmedTop(60).removeFromTop(32);
    const auto buttonWidth = 78;
    const auto gap = 14;
    auto buttons = buttonRow.withSizeKeepingCentre(buttonWidth * 2 + gap, 32);

    _yesButton.setBounds(buttons.removeFromLeft(buttonWidth));
    buttons.removeFromLeft(gap);
    _noButton.setBounds(buttons.removeFromLeft(buttonWidth));
}

void DeleteProfileConfirmationModule::confirm() {
    if (onConfirm) {
        onConfirm();
    }
}

void DeleteProfileConfirmationModule::cancel() {
    if (onCancel) {
        onCancel();
    }
}
