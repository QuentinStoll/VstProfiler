#include "Views/IrCapturePanel.h"

#include "PluginProcessor.h"
#include "Stylesheet.h"

namespace {
void wipeKey(std::array<std::uint8_t, 32>& key) {
    key.fill(0);
}
}  // namespace

IrCapturePanel::IrCapturePanel(ProfilerAudioProcessor& processor)
    : _processor(processor) {
    _title.setText("Clone IR", juce::dontSendNotification);
    _title.setFont(ProfilerStyle::Fonts::bold(20.0f));
    _title.setColour(juce::Label::textColourId, ProfilerStyle::Colors::text);
    addAndMakeVisible(_title);

    _body.setFont(ProfilerStyle::Fonts::regular(13.0f));
    _body.setColour(juce::Label::textColourId, ProfilerStyle::Colors::textMuted);
    _body.setJustificationType(juce::Justification::topLeft);
    addAndMakeVisible(_body);

    _nameLabel.setText("Name", juce::dontSendNotification);
    _nameLabel.setFont(ProfilerStyle::Fonts::regular(13.0f));
    _nameLabel.setColour(juce::Label::textColourId, ProfilerStyle::Colors::textMuted);
    addAndMakeVisible(_nameLabel);

    _nameEditor.setInputRestrictions(80);
    addAndMakeVisible(_nameEditor);

    _cloneButton.onClick = [this]() { startCapture(); };
    addAndMakeVisible(_cloneButton);

    _status.setFont(ProfilerStyle::Fonts::regular(13.0f));
    _status.setColour(juce::Label::textColourId, ProfilerStyle::Colors::accent);
    _status.setJustificationType(juce::Justification::topLeft);
    addAndMakeVisible(_status);

    refreshAccountState();
}

IrCapturePanel::~IrCapturePanel() {
    stopTimer();
}

void IrCapturePanel::paint(juce::Graphics& g) {
    g.setColour(ProfilerStyle::Colors::border.withAlpha(0.35f));
    g.drawHorizontalLine(0, 0.0f, (float)getWidth());
}

void IrCapturePanel::resized() {
    auto area = getLocalBounds();
    area.removeFromTop(16);
    _title.setBounds(area.removeFromTop(28));
    area.removeFromTop(8);
    _body.setBounds(area.removeFromTop(52));
    area.removeFromTop(12);
    auto line = area.removeFromTop(36);
    _nameLabel.setBounds(line.removeFromLeft(180));
    line.removeFromLeft(20);
    _nameEditor.setBounds(line.removeFromLeft(220));
    area.removeFromTop(16);
    _cloneButton.setBounds(area.removeFromTop(36).removeFromLeft(160));
    area.removeFromTop(12);
    _status.setBounds(area.removeFromTop(48));
}

void IrCapturePanel::refreshAccountState() {
    const auto standalone = _processor.wrapperType == juce::AudioProcessor::wrapperType_Standalone;
    Marketplace::Session session;
    const auto signedIn = Marketplace::loadSession(Marketplace::defaultSessionFile(), session) && session.accessToken.isNotEmpty();
    if (!standalone) {
        _body.setText("IR capture plays a sweep and records the return. Use the standalone, which owns the audio device.",
                      juce::dontSendNotification);
        _cloneButton.setEnabled(false);
        _status.setText("Open Profiler standalone to clone an IR.", juce::dontSendNotification);
        return;
    }

    _body.setText("The sweep stays in memory. The sealed IR is sent to your marketplace account and is not written to disk.",
                  juce::dontSendNotification);
    _cloneButton.setEnabled(signedIn && !_busy);
    if (!signedIn) {
        _status.setText("Sign in on the Library page to clone an IR.", juce::dontSendNotification);
    } else if (!_busy) {
        _status.setText("Signed in as " + session.email + ".", juce::dontSendNotification);
    }
}

void IrCapturePanel::startCapture() {
    if (_busy) {
        return;
    }

    refreshAccountState();
    if (!_cloneButton.isEnabled()) {
        return;
    }
    if (_nameEditor.getText().trim().isEmpty()) {
        _status.setText("Name the IR before cloning it.", juce::dontSendNotification);
        return;
    }

    juce::String error;
    if (!_processor.beginIrCapture(&error)) {
        _status.setText(error, juce::dontSendNotification);
        return;
    }

    _busy = true;
    _cloneButton.setEnabled(false);
    _status.setText("Playing the sweep. Keep the return connected.", juce::dontSendNotification);
    startTimerHz(15);
}

void IrCapturePanel::timerCallback() {
    if (!_processor.irCaptureFinished()) {
        return;
    }
    stopTimer();
    sealAndUpload();
}

void IrCapturePanel::sealAndUpload() {
    Marketplace::Session session;
    if (!Marketplace::loadSession(Marketplace::defaultSessionFile(), session) || session.accessToken.isEmpty()) {
        _busy = false;
        _status.setText("Sign in on the Library page to clone an IR.", juce::dontSendNotification);
        refreshAccountState();
        return;
    }

    if (!_processor.unlockSpectraSession(session.accessToken)) {
        _busy = false;
        _status.setText("Could not open a capture session.", juce::dontSendNotification);
        refreshAccountState();
        return;
    }

    juce::MemoryBlock sealed;
    std::array<std::uint8_t, 32> key{};
    juce::String error;
    const auto sealedOk = _processor.sealIrCapture(sealed, key, &error);
    _processor.lockSpectraSession();
    if (!sealedOk) {
        wipeKey(key);
        sealed.fillWith(0);
        _busy = false;
        _status.setText(error, juce::dontSendNotification);
        refreshAccountState();
        return;
    }

    const auto title = _nameEditor.getText().trim();
    const auto token = session.accessToken;
    juce::Component::SafePointer<IrCapturePanel> safeThis(this);
    _jobs.addJob([safeThis, title, token, sealed, key]() mutable {
        Marketplace::Client client;
        const auto result = client.submitIrCapture(token, title, key, sealed);
        wipeKey(key);
        sealed.fillWith(0);
        juce::MessageManager::callAsync([safeThis, result]() {
            if (safeThis == nullptr) {
                return;
            }
            safeThis->_busy = false;
            safeThis->_status.setText(result.message, juce::dontSendNotification);
            safeThis->refreshAccountState();
        });
    });
    wipeKey(key);
    sealed.fillWith(0);
}
