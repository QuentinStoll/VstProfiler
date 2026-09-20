#include "Components/SignalChainStrip.h"

#include <cmath>

namespace {
constexpr int kSignalChainRadioGroup = 0x50524F46;  // "PROF"
constexpr float kMeterFloorDb = -54.0f;
constexpr float kMeterCeilDb = -3.0f;
constexpr float kMeterAttackSeconds = 0.045f;
constexpr float kMeterReleaseSeconds = 0.22f;

float meterFromDb(float db) {
    const auto normalised = juce::jmap(juce::jlimit(kMeterFloorDb, kMeterCeilDb, db),
                                       kMeterFloorDb, kMeterCeilDb, 0.0f, 1.0f);
    return std::pow(normalised, 0.62f);
}

float smoothMeter(float current, float target, float deltaSeconds) {
    const auto tau = target > current ? kMeterAttackSeconds : kMeterReleaseSeconds;
    const auto coeff = 1.0f - std::exp(-deltaSeconds / tau);
    return current + (target - current) * coeff;
}
}

SignalChainBlock::SignalChainBlock(juce::Colour categoryColour, CustomLookAndFeel::RigIcon icon)
    : juce::Button({}),
      _categoryColour(categoryColour),
      _icon(icon) {
    setClickingTogglesState(true);
    setRadioGroupId(kSignalChainRadioGroup);
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
    setOpaque(true);
}

void SignalChainBlock::setLedOn(bool shouldBeOn) {
    if (_ledOn == shouldBeOn) {
        return;
    }

    _ledOn = shouldBeOn;
    repaint();
}

void SignalChainBlock::setIoNode(bool isIoNode) {
    _isIoNode = isIoNode;
    setOpaque(!isIoNode);
    repaint();
}

void SignalChainBlock::setShowsLed(bool shouldShowLed) {
    _showsLed = shouldShowLed;
    repaint();
}

void SignalChainBlock::setSignalLevel(float level) {
    const auto clamped = juce::jlimit(0.0f, 1.0f, level);
    if (std::abs(_signalLevel - clamped) < 0.001f) {
        return;
    }

    _signalLevel = clamped;
    if (_isIoNode) {
        repaint();
    }
}

juce::Rectangle<float> SignalChainBlock::getLedBounds() const {
    const auto bounds = getLocalBounds().toFloat();
    return _isIoNode ? CustomLookAndFeel::getSignalIoLedBounds(bounds)
                     : CustomLookAndFeel::getSignalChainLedBounds(bounds);
}

bool SignalChainBlock::isLedHit(juce::Point<int> position) const {
    if (!_showsLed) {
        return false;
    }

    return getLedBounds().expanded(4.0f).contains(position.toFloat());
}

bool SignalChainBlock::hitTest(int x, int y) {
    if (isLedHit({x, y})) {
        return true;
    }

    if (!_isIoNode) {
        return juce::Button::hitTest(x, y);
    }

    const auto bounds = getLocalBounds().toFloat();
    const auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.31f;
    return bounds.getCentre().getDistanceFrom({static_cast<float>(x), static_cast<float>(y)}) <= radius + 3.0f;
}

void SignalChainBlock::mouseUp(const juce::MouseEvent& event) {
    if (event.mouseWasClicked() && isLedHit(event.getPosition()) && onLedClicked) {
        onLedClicked();
        return;
    }

    juce::Button::mouseUp(event);
}

void SignalChainBlock::paint(juce::Graphics& g) {
    paintContents(g, isMouseOverOrDragging());
}

void SignalChainBlock::paintButton(juce::Graphics&, bool, bool) {}

void SignalChainBlock::paintContents(juce::Graphics& g, bool isMouseOverButton) {
    if (!_isIoNode) {
        g.fillAll(juce::Colours::black);
    }

    if (auto* laf = dynamic_cast<CustomLookAndFeel*>(&getLookAndFeel())) {
        if (_isIoNode) {
            laf->drawSignalIoNode(g, getLocalBounds().toFloat(), getToggleState(), isMouseOverButton, _ledOn, _showsLed, _signalLevel);
            return;
        }

        laf->drawSignalChainBlock(g,
                                  getLocalBounds().toFloat(),
                                  _categoryColour,
                                  _icon,
                                  getToggleState(),
                                  isMouseOverButton,
                                  _ledOn);
        return;
    }

    ProfilerStyle::Surfaces::fillPanel(g, getLocalBounds().toFloat());
}

void SignalChainStrip::SignalBusLayer::setLine(float y, float x1, float x2) {
    _y = y;
    _x1 = x1;
    _x2 = x2;
}

void SignalChainStrip::SignalBusLayer::paint(juce::Graphics& g) {
    if (auto* laf = dynamic_cast<CustomLookAndFeel*>(&getLookAndFeel())) {
        laf->drawSignalBus(g, _y, _x1, _x2);
        return;
    }

    g.setColour(ProfilerStyle::Colors::caption);
    g.drawLine(_x1, _y, _x2, _y, 1.2f);
}

SignalChainStrip::SignalChainStrip() {
    addAndMakeVisible(_busLayer);
    _busLayer.setInterceptsMouseClicks(false, false);

    auto configure = [this](SignalChainBlock& block, BlockId id) {
        addAndMakeVisible(block);
        block.onClick = [this, id]() {
            handleBlockClick(id);
        };
        block.onLedClicked = [this, id]() {
            setSelectedBlock(id);
            if (onBlockBypassToggled) {
                onBlockBypassToggled(id);
            }
        };
    };

    configure(_inputGate, BlockId::InputGate);
    configure(_ampProfiler, BlockId::AmpProfiler);
    configure(_cabinetIr, BlockId::CabinetIr);
    configure(_eqPostFx, BlockId::EqPostFx);
    configure(_masterVolume, BlockId::MasterVolume);

    _inputGate.setIoNode(true);
    _masterVolume.setIoNode(true);
    _masterVolume.setShowsLed(false);
    _ampProfiler.setToggleState(true, juce::dontSendNotification);
    _busLayer.toBack();
    setWantsKeyboardFocus(true);
}

void SignalChainStrip::paint(juce::Graphics& g) {
    auto* laf = dynamic_cast<CustomLookAndFeel*>(&getLookAndFeel());

    for (int slot = 0; slot < kSlotCount; ++slot) {
        if (isOccupiedSlot(slot) || _slotBounds[static_cast<size_t>(slot)].isEmpty()) {
            continue;
        }

        if (laf != nullptr) {
            laf->drawEmptySignalSlot(g, _slotBounds[static_cast<size_t>(slot)].toFloat());
        } else {
            g.setColour(ProfilerStyle::Colors::border.withAlpha(0.55f));
            auto outline = _slotBounds[static_cast<size_t>(slot)].toFloat();
            const auto side = juce::jmin(outline.getWidth(), outline.getHeight()) / 2.5f;
            g.drawRoundedRectangle(outline.withSizeKeepingCentre(side, side), 3.0f, 1.2f);
        }
    }
}

void SignalChainStrip::resized() {
    auto area = getLocalBounds();
    const auto slotWidth = area.getWidth() / static_cast<float>(kSlotCount);
    const auto square = juce::jlimit(100, 110, juce::roundToInt(slotWidth) - 16);
    const auto rowY = area.getY() + juce::jmax(0, (area.getHeight() - square) / 2);

    for (int slot = 0; slot < kSlotCount; ++slot) {
        const auto centreX = juce::roundToInt(static_cast<float>(area.getX()) + slotWidth * (static_cast<float>(slot) + 0.5f));
        _slotBounds[static_cast<size_t>(slot)] = {centreX - square / 2, rowY, square, square};
    }

    SignalChainBlock* blocks[] = {&_inputGate, &_ampProfiler, &_cabinetIr, &_eqPostFx, &_masterVolume};
    const auto ioHit = juce::jlimit(36, 44, juce::roundToInt(static_cast<float>(square) * 0.38f));
    for (int index = 0; index < 5; ++index) {
        const auto slot = _slotBounds[static_cast<size_t>(kOccupiedSlots[index])];
        if (blocks[index]->isIoNode()) {
            blocks[index]->setBounds(slot.withSizeKeepingCentre(ioHit, ioHit));
        } else {
            blocks[index]->setBounds(slot);
        }
    }

    updateBusLayer();
    _busLayer.toBack();
}

void SignalChainStrip::updateBusLayer() {
    _busLayer.setBounds(getLocalBounds());

    const auto inputCentre = _inputGate.getBounds().toFloat().getCentre();
    const auto outputCentre = _masterVolume.getBounds().toFloat().getCentre();
    const auto busY = inputCentre.isOrigin()
                          ? static_cast<float>(getLocalBounds().getCentreY())
                          : inputCentre.y;
    const auto busStartX = inputCentre.isOrigin() ? 0.0f : inputCentre.x;
    const auto busEndX = outputCentre.isOrigin() ? static_cast<float>(getWidth()) : outputCentre.x;
    _busLayer.setLine(busY, busStartX, busEndX);
}

void SignalChainStrip::setSelectedBlock(BlockId blockId) {
    handleBlockClick(blockId);
    getBlock(blockId).setToggleState(true, juce::dontSendNotification);
}

bool SignalChainStrip::selectAdjacentBlock(int delta) {
    if (delta == 0) {
        return false;
    }

    constexpr int blockCount = 5;
    const auto current = static_cast<int>(_selected);
    const auto next = (current + delta + blockCount) % blockCount;
    setSelectedBlock(static_cast<BlockId>(next));
    return true;
}

bool SignalChainStrip::keyPressed(const juce::KeyPress& key) {
    if (key == juce::KeyPress::leftKey || key == juce::KeyPress::upKey) {
        return selectAdjacentBlock(-1);
    }

    if (key == juce::KeyPress::rightKey || key == juce::KeyPress::downKey) {
        return selectAdjacentBlock(1);
    }

    return false;
}

void SignalChainStrip::setBlockLed(BlockId blockId, bool isOn) {
    getBlock(blockId).setLedOn(isOn);
}

void SignalChainStrip::setIoMeterLevels(float inputDb, float outputDb) {
    const auto nowMs = juce::Time::getMillisecondCounterHiRes();
    const auto deltaSeconds = _lastMeterMs > 0.0
                                  ? static_cast<float>(juce::jlimit(0.001, 0.05, (nowMs - _lastMeterMs) * 0.001))
                                  : (1.0f / 60.0f);
    _lastMeterMs = nowMs;

    _inputMeter = smoothMeter(_inputMeter, meterFromDb(inputDb), deltaSeconds);
    _outputMeter = smoothMeter(_outputMeter, meterFromDb(outputDb), deltaSeconds);
    _inputGate.setSignalLevel(_inputMeter);
    _masterVolume.setSignalLevel(_outputMeter);
}

SignalChainBlock& SignalChainStrip::getBlock(BlockId blockId) {
    switch (blockId) {
        case BlockId::InputGate:
            return _inputGate;
        case BlockId::CabinetIr:
            return _cabinetIr;
        case BlockId::EqPostFx:
            return _eqPostFx;
        case BlockId::MasterVolume:
            return _masterVolume;
        case BlockId::AmpProfiler:
        default:
            return _ampProfiler;
    }
}

const SignalChainBlock& SignalChainStrip::getBlock(BlockId blockId) const {
    return const_cast<SignalChainStrip*>(this)->getBlock(blockId);
}

void SignalChainStrip::handleBlockClick(BlockId blockId) {
    _selected = blockId;
    if (onBlockSelected) {
        onBlockSelected(blockId);
    }
}

bool SignalChainStrip::isOccupiedSlot(int slot) const {
    for (const auto occupied : kOccupiedSlots) {
        if (occupied == slot) {
            return true;
        }
    }

    return false;
}
