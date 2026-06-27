#include "Modules/CreateProfilModule.h"

#include "BinaryData.h"
#include "Stylesheet.h"

CreateProfilModule::CreateProfilModule() {
    _musicIcon = juce::Drawable::createFromImageData(BinaryData::musicnote_svg,
                                                     BinaryData::musicnote_svgSize);
    if (_musicIcon != nullptr) {
        _musicIcon->replaceColour(juce::Colour(0xff000000), ProfilerStyle::Colors::white);
    }

    addAndMakeVisible(_profileNameEditor);
    addAndMakeVisible(_createButton);
    addAndMakeVisible(_cancelButton);
    addAndMakeVisible(_form);

    _profileNameEditor.setText("Profil 1", juce::dontSendNotification);
    _createButton.onClick = [this]() {
        createProfil();
    };
    _cancelButton.onClick = [this]() {
        cancel();
    };

    _form.setLabelWidth(176);
    _form.setRowGap(12);

    _form.addSliderField("masterVolume", "Master Volume", 0.0, 1.0, 0.68, "%", 0.1);
    _form.addSliderField("gain", "Gain", 0.0, 1.0, 0.68, "dB", 0.1);
    _form.addSliderField("noiseGate", "Noise gate", 0.0, 1.0, 0.68, "dB", 0.1);
    _form.addSliderField("bass", "Bass", 0.0, 1.0, 0.68, "dB", 0.1);
    _form.addSliderField("middle", "Middle", 0.0, 1.0, 0.68, "dB", 0.1);
    _form.addSliderField("treble", "Treble", 0.0, 1.0, 0.68, "dB", 0.1);
    _form.addSliderField("presence", "Presence", 0.0, 1.0, 0.68, "dB", 0.1);
    _form.addSliderField("depth", "Depth", 0.0, 1.0, 0.68, "dB", 0.1);
    _form.addPathField("irPath", "IR Path", "*.wav;*.aiff;*.aif;*.flac");
    _form.addPathField("ampPath", "Amp Path", "*.nam;*.json;*.txt");
}

void CreateProfilModule::paint(juce::Graphics& g) {
    if (_musicIcon != nullptr) {
        _musicIcon->drawWithin(g,
                               juce::Rectangle<float>(10.0f, 14.0f, 28.0f, 28.0f),
                               juce::RectanglePlacement::centred,
                               1.0f);
    }
}

void CreateProfilModule::resized() {
    auto area = getLocalBounds();
    auto topRow = area.removeFromTop(58).reduced(0, 15);

    topRow.removeFromLeft(55);
    _profileNameEditor.setBounds(topRow.removeFromLeft(250));
    topRow.removeFromLeft(14);
    _createButton.setBounds(topRow.removeFromLeft(76));
    topRow.removeFromLeft(5);
    _cancelButton.setBounds(topRow.removeFromLeft(76));

    auto formArea = getLocalBounds()
                        .withTrimmedTop(88)
                        .withTrimmedLeft(50)
                        .withTrimmedRight(44);

    _form.setBounds(formArea.removeFromTop(juce::jmin(formArea.getHeight(), _form.getContentHeight())));
}

int CreateProfilModule::getRequiredHeight(int /*width*/) const {
    return 88 + _form.getContentHeight() + 20;
}

void CreateProfilModule::createProfil() {
    auto values = _form.getValues();
    values.set("profileName", _profileNameEditor.getText());

    if (onCreateClicked) {
        onCreateClicked(values);
    }
}

void CreateProfilModule::cancel() {
    if (onCancelClicked) {
        onCancelClicked();
    }
}
