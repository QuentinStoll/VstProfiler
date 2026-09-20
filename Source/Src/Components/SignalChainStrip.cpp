#include "Components/SignalChainStrip.h"

namespace {
constexpr int kSignalChainRadioGroup = 0x50524F46;  // "PROF"
}

SignalChainBlock::SignalChainBlock(const juce::String& title,
                                   const juce::String& subtitle,
                                   juce::Colour categoryColour,
                                   CustomLookAndFeel::RigIcon icon)
    : juce::Button(title),
      _title(title),
      _subtitle(subtitle),
      _categoryColour(categoryColour),
      _icon(icon) {
    setClickingTogglesState(true);
    setRadioGroupId(kSignalChainRadioGroup);
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void SignalChainBlock::setSubtitle(const juce::String& subtitle) {
    if (_subtitle == subtitle) {
        return;
    }

    _subtitle = subtitle;
    repaint();
}

void SignalChainBlock::setLedOn(bool shouldBeOn) {
    if (_isIoNode || _ledOn == shouldBeOn) {
        return;
    }

    _ledOn = shouldBeOn;
    repaint();
}

void SignalChainBlock::setIoNode(bool isIoNode) {
    _isIoNode = isIoNode;
    setOpaque(false);
    repaint();
}

bool SignalChainBlock::hitTest(int x, int y) {
    if (!_isIoNode) {
        return juce::Button::hitTest(x, y);
    }

    const auto bounds = getLocalBounds().toFloat();
    const auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.31f;
    return bounds.getCentre().getDistanceFrom({static_cast<float>(x), static_cast<float>(y)}) <= radius + 3.0f;
}

void SignalChainBlock::paintButton(juce::Graphics& g, bool isMouseOverButton, bool /*isButtonDown*/) {
    if (auto* laf = dynamic_cast<CustomLookAndFeel*>(&getLookAndFeel())) {
        if (_isIoNode) {
            laf->drawSignalIoNode(g, getLocalBounds().toFloat(), getToggleState(), isMouseOverButton);
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

SignalChainStrip::SignalChainStrip() {
    auto configure = [this](SignalChainBlock& block, BlockId id) {
        addAndMakeVisible(block);
        block.onClick = [this, id]() {
            handleBlockClick(id);
        };
    };

    configure(_inputGate, BlockId::InputGate);
    configure(_ampProfiler, BlockId::AmpProfiler);
    configure(_cabinetIr, BlockId::CabinetIr);
    configure(_eqPostFx, BlockId::EqPostFx);
    configure(_masterVolume, BlockId::MasterVolume);

    _inputGate.setIoNode(true);
    _masterVolume.setIoNode(true);
    _ampProfiler.setToggleState(true, juce::dontSendNotification);
}

void SignalChainStrip::paint(juce::Graphics& g) {
    auto* laf = dynamic_cast<CustomLookAndFeel*>(&getLookAndFeel());
    const auto inputCentre = _inputGate.getBounds().toFloat().getCentre();
    const auto outputCentre = _masterVolume.getBounds().toFloat().getCentre();
    const auto busY = inputCentre.isOrigin()
                          ? static_cast<float>(getLocalBounds().getCentreY())
                          : inputCentre.y;
    const auto busStartX = inputCentre.isOrigin() ? 0.0f : inputCentre.x;
    const auto busEndX = outputCentre.isOrigin() ? static_cast<float>(getWidth()) : outputCentre.x;

    if (laf != nullptr) {
        laf->drawSignalBus(g, busY, busStartX, busEndX);
    } else {
        g.setColour(ProfilerStyle::Colors::caption);
        g.drawLine(busStartX, busY, busEndX, busY, 1.2f);
    }

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
}

void SignalChainStrip::setSelectedBlock(BlockId blockId) {
    handleBlockClick(blockId);
    getBlock(blockId).setToggleState(true, juce::dontSendNotification);
}

void SignalChainStrip::setBlockSubtitle(BlockId blockId, const juce::String& subtitle) {
    getBlock(blockId).setSubtitle(subtitle);
}

void SignalChainStrip::setBlockLed(BlockId blockId, bool isOn) {
    getBlock(blockId).setLedOn(isOn);
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
