#include "Modules/EditProfilModule.h"

#include "BinaryData.h"
#include "Stylesheet.h"

EditProfilModule::EditProfilModule() {
    _musicIcon = juce::Drawable::createFromImageData(BinaryData::musicnote_svg,
                                                     BinaryData::musicnote_svgSize);
    if (_musicIcon != nullptr) {
        _musicIcon->replaceColour(juce::Colour(0xff000000), ProfilerStyle::Colors::white);
    }

    addAndMakeVisible(_profileNameEditor);
    addAndMakeVisible(_saveButton);
    addAndMakeVisible(_deleteButton);
    addAndMakeVisible(_backButton);
    addAndMakeVisible(_form);

    _saveButton.onClick = [this]() {
        saveProfil();
    };
    _deleteButton.onClick = [this]() {
        deleteProfil();
    };
    _backButton.onClick = [this]() {
        back();
    };

    _form.setLabelWidth(114);
    _form.setRowGap(10);

    _form.addSliderField("masterVolume", "Master Volume", 0.0, 100.0, 50.0, "%", 1.0);
    _form.addSliderField("gain", "Gain", -12.0, 12.0, 0.0, "dB", 0.1);
    _form.addSliderField("noiseGate", "Noise gate", 0.0, 60.0, 10.0, "dB", 0.1);
    _form.addSliderField("bass", "Bass", -24.0, 24.0, 0.0, "dB", 0.1);
    _form.addSliderField("middle", "Middle", -24.0, 24.0, 0.0, "dB", 0.1);
    _form.addSliderField("treble", "Treble", -24.0, 24.0, 0.0, "dB", 0.1);
    _form.addSliderField("presence", "Presence", -24.0, 24.0, 0.0, "dB", 0.1);
    _form.addSliderField("depth", "Depth", -24.0, 24.0, 0.0, "dB", 0.1);
    _form.addPathField("irPath", "IR Path", "*.wav;*.aiff;*.aif;*.flac");
    _form.addPathField("ampPath", "Amp Path", "*.nam;*.json;*.txt");
}

void EditProfilModule::setProfileNumber(int profileNumber) {
    setProfile(profileNumber, {});
}

void EditProfilModule::setProfile(int profileNumber, const juce::NamedValueSet& values) {
    _profileNumber = profileNumber;
    _profileNameEditor.setText("Profil " + juce::String(_profileNumber), juce::dontSendNotification);
    _form.setValues(values);

    if (const auto* profileName = values.getVarPointer("profileName")) {
        _profileNameEditor.setText(profileName->toString(), juce::dontSendNotification);
    }
}

int EditProfilModule::getProfileNumber() const {
    return _profileNumber;
}

void EditProfilModule::paint(juce::Graphics& g) {
    if (_musicIcon != nullptr) {
        _musicIcon->drawWithin(g,
                               juce::Rectangle<float>(8.0f, 8.0f, 26.0f, 26.0f),
                               juce::RectanglePlacement::centred,
                               1.0f);
    }
}

void EditProfilModule::resized() {
    auto area = getLocalBounds();
    auto topRow = area.removeFromTop(40).reduced(0, 8);

    topRow.removeFromLeft(40);
    _profileNameEditor.setBounds(topRow.removeFromLeft(176));
    topRow.removeFromLeft(14);
    _saveButton.setBounds(topRow.removeFromLeft(40));
    topRow.removeFromLeft(5);
    _deleteButton.setBounds(topRow.removeFromLeft(55));
    topRow.removeFromLeft(5);
    _backButton.setBounds(topRow.removeFromLeft(42));

    auto formArea = getLocalBounds()
                        .withTrimmedTop(58)
                        .withTrimmedLeft(26)
                        .withTrimmedRight(28);

    _form.setBounds(formArea.removeFromTop(juce::jmin(formArea.getHeight(), _form.getContentHeight())));
}

int EditProfilModule::getRequiredHeight(int /*width*/) const {
    return 58 + _form.getContentHeight() + 18;
}

void EditProfilModule::saveProfil() {
    auto values = _form.getValues();
    values.set("profileName", _profileNameEditor.getText());

    if (onSaveClicked) {
        onSaveClicked(_profileNumber, values);
    }
}

void EditProfilModule::deleteProfil() {
    if (onDeleteClicked) {
        onDeleteClicked(_profileNumber);
    }
}

void EditProfilModule::back() {
    if (onBackClicked) {
        onBackClicked();
    }
}
