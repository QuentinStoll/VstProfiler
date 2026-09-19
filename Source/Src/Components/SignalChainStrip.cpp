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
                                  _title,
                                  _subtitle,
                                  _categoryColour,
                                  _icon,
                                  getToggleState(),
                                  isMouseOverButton,
                                  _ledOn);
        return;
    }

    ProfilerStyle::Surfaces::fillPanel(g, getLocalBounds().toFloat());
    g.setColour(ProfilerStyle::Colors::text);
    g.drawFittedText(_title, getLocalBounds().reduced(8), juce::Justification::centred, 1);
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
    const SignalChainBlock* blocks[] = {&_inputGate, &_ampProfiler, &_cabinetIr, &_eqPostFx};

    for (int index = 0; index < 3; ++index) {
        const auto from = blocks[index]->getBounds().toFloat().getCentre().withX(blocks[index]->getBounds().toFloat().getRight());
        const auto to = blocks[index + 1]->getBounds().toFloat().getCentre().withX(blocks[index + 1]->getBounds().toFloat().getX());

        if (laf != nullptr) {
            laf->drawSignalCable(g, from, to, true);
        } else {
            g.setColour(ProfilerStyle::Colors::caption);
            g.drawLine(from.x, from.y, to.x, to.y, 1.2f);
        }
    }
}

void SignalChainStrip::resized() {
    auto area = getLocalBounds();
    const auto count = 4;
    const auto blockHeight = juce::jlimit(124, 188, area.getHeight() - 12);
    const auto blockWidth = juce::jlimit(112, 152, juce::roundToInt(blockHeight * 0.78f) + 18);
    const auto gap = juce::jmax(28, juce::roundToInt(area.getWidth() * 0.032f));
    const auto totalWidth = count * blockWidth + (count - 1) * gap;
    auto row = juce::Rectangle<int>(totalWidth, blockHeight).withCentre(area.getCentre());

    _inputGate.setBounds(row.removeFromLeft(blockWidth));
    row.removeFromLeft(gap);
    _ampProfiler.setBounds(row.removeFromLeft(blockWidth));
    row.removeFromLeft(gap);
    _cabinetIr.setBounds(row.removeFromLeft(blockWidth));
    row.removeFromLeft(gap);
    _eqPostFx.setBounds(row.removeFromLeft(blockWidth));
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
