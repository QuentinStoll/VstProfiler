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
    setTooltip(title);
}

void SignalChainBlock::setSubtitle(const juce::String& subtitle) {
    if (_subtitle == subtitle) {
        return;
    }

    _subtitle = subtitle;
    repaint();
}

void SignalChainBlock::setLedOn(bool shouldBeOn) {
    if (_ledOn == shouldBeOn) {
        return;
    }

    _ledOn = shouldBeOn;
    repaint();
}

void SignalChainBlock::paintButton(juce::Graphics& g, bool isMouseOverButton, bool /*isButtonDown*/) {
    if (auto* laf = dynamic_cast<CustomLookAndFeel*>(&getLookAndFeel())) {
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

    _ampProfiler.setToggleState(true, juce::dontSendNotification);
}

void SignalChainStrip::paint(juce::Graphics& g) {
    auto* laf = dynamic_cast<CustomLookAndFeel*>(&getLookAndFeel());
    const auto busY = _slotBounds.front().isEmpty()
                          ? static_cast<float>(getLocalBounds().getCentreY())
                          : _slotBounds.front().toFloat().getCentreY();

    if (laf != nullptr) {
        laf->drawSignalBus(g, busY, 0.0f, static_cast<float>(getWidth()));
    } else {
        g.setColour(ProfilerStyle::Colors::caption);
        g.drawLine(0.0f, busY, static_cast<float>(getWidth()), busY, 1.2f);
    }

    for (int slot = 0; slot < kSlotCount; ++slot) {
        const auto occupied = slot == kOccupiedSlots[0] || slot == kOccupiedSlots[1]
                              || slot == kOccupiedSlots[2] || slot == kOccupiedSlots[3];
        if (occupied || _slotBounds[static_cast<size_t>(slot)].isEmpty()) {
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

    SignalChainBlock* blocks[] = {&_inputGate, &_ampProfiler, &_cabinetIr, &_eqPostFx};
    for (int index = 0; index < 4; ++index) {
        blocks[index]->setBounds(_slotBounds[static_cast<size_t>(kOccupiedSlots[index])]);
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
