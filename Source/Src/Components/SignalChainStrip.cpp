#include "Components/SignalChainStrip.h"

#include <cmath>
#include <limits>

namespace {
constexpr int kSignalChainRadioGroup = 0x50524F46;  // "PROF"
constexpr float kMeterFloorDb = -54.0f;
constexpr float kMeterCeilDb = -3.0f;
constexpr float kMeterAttackSeconds = 0.045f;
constexpr float kMeterReleaseSeconds = 0.22f;
constexpr int kDragThresholdPx = 6;

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
    setMouseCursor(_isIoNode ? juce::MouseCursor::PointingHandCursor
                             : juce::MouseCursor::DraggingHandCursor);
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

void SignalChainBlock::mouseDown(const juce::MouseEvent& event) {
    if (event.mods.isPopupMenu()) {
        return;
    }

    juce::Button::mouseDown(event);
    if (_isIoNode || isLedHit(event.getPosition()) || onDragBegin == nullptr) {
        return;
    }

    onDragBegin(event);
}

void SignalChainBlock::mouseDrag(const juce::MouseEvent& event) {
    if (event.mods.isPopupMenu()) {
        return;
    }

    juce::Button::mouseDrag(event);
    if (onDragMove != nullptr) {
        onDragMove(event);
    }
}

void SignalChainBlock::mouseUp(const juce::MouseEvent& event) {
    if (event.mods.isPopupMenu()) {
        if (event.mouseWasClicked() && onPickerRequested) {
            onPickerRequested();
        }
        return;
    }

    if (onDragEnd != nullptr) {
        onDragEnd(event);
    }

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

    auto configure = [this](SignalChainBlock& block, BlockId id, bool movable) {
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
        if (!movable) {
            return;
        }

        block.onPickerRequested = [this, id]() {
            requestPicker(slotForBlock(id));
        };
        block.onDragBegin = [this, id](const juce::MouseEvent& event) {
            startBlockDrag(id, event);
        };
        block.onDragMove = [this](const juce::MouseEvent& event) {
            updateBlockDrag(event);
        };
        block.onDragEnd = [this](const juce::MouseEvent& event) {
            finishBlockDrag(event);
        };
    };

    configure(_inputGate, BlockId::InputGate, false);
    configure(_ampProfiler, BlockId::AmpProfiler, true);
    configure(_cabinetIr, BlockId::CabinetIr, true);
    configure(_eqPostFx, BlockId::EqPostFx, true);
    configure(_pedalDrive, BlockId::PedalDrive, true);
    configure(_masterVolume, BlockId::MasterVolume, false);

    _inputGate.setIoNode(true);
    _masterVolume.setIoNode(true);
    _masterVolume.setShowsLed(false);
    _ampProfiler.setToggleState(true, juce::dontSendNotification);
    _busLayer.toBack();
    setOpaque(true);
    setWantsKeyboardFocus(true);
}

void SignalChainStrip::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::black);
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

    if (_dropSlot >= SignalChain::firstMovableSlot && _dropSlot <= SignalChain::lastMovableSlot) {
        auto dropBounds = _slotBounds[static_cast<size_t>(_dropSlot)].toFloat().reduced(4.0f);
        g.setColour(ProfilerStyle::Colors::text.withAlpha(0.72f));
        g.drawRoundedRectangle(dropBounds, 6.0f, 1.6f);
    }
}

void SignalChainStrip::resized() {
    auto area = getLocalBounds();
    const auto slotWidth = area.getWidth() / static_cast<float>(kSlotCount);
    constexpr float kBlockScale = 0.75f;
    const auto square = juce::roundToInt(static_cast<float>(juce::jlimit(100, 110, juce::roundToInt(slotWidth) - 16)) * kBlockScale);
    const auto rowY = area.getY() + juce::jmax(0, (area.getHeight() - square) / 2);

    for (int slot = 0; slot < kSlotCount; ++slot) {
        const auto centreX = juce::roundToInt(static_cast<float>(area.getX()) + slotWidth * (static_cast<float>(slot) + 0.5f));
        _slotBounds[static_cast<size_t>(slot)] = {centreX - square / 2, rowY, square, square};
    }

    placeBlocks();
    updateBusLayer();
    _busLayer.toBack();
}

void SignalChainStrip::mouseUp(const juce::MouseEvent& event) {
    if (!event.mods.isPopupMenu() || !event.mouseWasClicked()) {
        return;
    }

    const auto slot = slotAtPosition(event.getPosition());
    if (slot >= SignalChain::firstMovableSlot && slot <= SignalChain::lastMovableSlot) {
        requestPicker(slot);
    }
}

void SignalChainStrip::placeBlocks() {
    SignalChainBlock* blocks[] = {
        &_inputGate, &_ampProfiler, &_cabinetIr, &_eqPostFx, &_pedalDrive, &_masterVolume};
    const BlockId ids[] = {
        BlockId::InputGate, BlockId::AmpProfiler, BlockId::CabinetIr,
        BlockId::EqPostFx, BlockId::PedalDrive, BlockId::MasterVolume};
    const auto square = _slotBounds.front().getWidth();
    const auto ioHit = juce::jlimit(26, 34, juce::roundToInt(static_cast<float>(square) * 0.38f));

    for (int index = 0; index < 6; ++index) {
        const auto slotIndex = slotForBlock(ids[index]);
        const auto onChain = slotIndex >= 0;
        blocks[index]->setVisible(onChain);
        if (!onChain || (_dragActive && ids[index] == _dragBlock)) {
            continue;
        }

        const auto slot = _slotBounds[static_cast<size_t>(slotIndex)];
        if (blocks[index]->isIoNode()) {
            blocks[index]->setBounds(slot.withSizeKeepingCentre(ioHit, ioHit));
        } else {
            blocks[index]->setBounds(slot);
        }
    }
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
    if (!isBlockOnChain(blockId)) {
        return;
    }

    handleBlockClick(blockId);
    getBlock(blockId).setToggleState(true, juce::dontSendNotification);
}

bool SignalChainStrip::selectAdjacentBlock(int delta) {
    if (delta == 0) {
        return false;
    }

    std::array<BlockId, 8> order{};
    const auto blockCount = fillVisualOrder(order);
    if (blockCount <= 0) {
        return false;
    }

    int currentIndex = 0;
    for (int index = 0; index < blockCount; ++index) {
        if (order[static_cast<size_t>(index)] == _selected) {
            currentIndex = index;
            break;
        }
    }

    const auto next = (currentIndex + delta % blockCount + blockCount) % blockCount;
    setSelectedBlock(order[static_cast<size_t>(next)]);
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

void SignalChainStrip::setLayout(const SignalChain::Layout& layout) {
    _layout = layout.isValid() ? layout : SignalChain::Layout{};
    if (!isBlockOnChain(_selected)) {
        _selected = BlockId::InputGate;
        _inputGate.setToggleState(true, juce::dontSendNotification);
        if (onBlockSelected) {
            onBlockSelected(_selected);
        }
    }
    placeBlocks();
    updateBusLayer();
    repaint();
}

bool SignalChainStrip::isBlockOnChain(BlockId blockId) const noexcept {
    return slotForBlock(blockId) >= 0;
}

SignalChainBlock& SignalChainStrip::getBlock(BlockId blockId) {
    switch (blockId) {
        case BlockId::InputGate:
            return _inputGate;
        case BlockId::CabinetIr:
            return _cabinetIr;
        case BlockId::EqPostFx:
            return _eqPostFx;
        case BlockId::PedalDrive:
            return _pedalDrive;
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
    if (_dragActive && slot == slotForBlock(_dragBlock)) {
        return false;
    }

    return _layout.occupies(slot);
}

int SignalChainStrip::slotForBlock(BlockId blockId) const noexcept {
    switch (blockId) {
        case BlockId::InputGate:
            return SignalChain::inputSlot;
        case BlockId::MasterVolume:
            return SignalChain::outputSlot;
        case BlockId::AmpProfiler:
            return _layout.slotFor(SignalChain::Stage::Amp);
        case BlockId::CabinetIr:
            return _layout.slotFor(SignalChain::Stage::Cab);
        case BlockId::EqPostFx:
            return _layout.slotFor(SignalChain::Stage::Eq);
        case BlockId::PedalDrive:
            return _layout.slotFor(SignalChain::Stage::Pedal);
        default:
            return -1;
    }
}

SignalChain::Stage SignalChainStrip::stageForBlock(BlockId blockId) noexcept {
    switch (blockId) {
        case BlockId::CabinetIr:
            return SignalChain::Stage::Cab;
        case BlockId::EqPostFx:
            return SignalChain::Stage::Eq;
        case BlockId::PedalDrive:
            return SignalChain::Stage::Pedal;
        case BlockId::AmpProfiler:
        default:
            return SignalChain::Stage::Amp;
    }
}

SignalChainStrip::BlockId SignalChainStrip::blockForStage(SignalChain::Stage stage) noexcept {
    switch (stage) {
        case SignalChain::Stage::Cab:
            return BlockId::CabinetIr;
        case SignalChain::Stage::Eq:
            return BlockId::EqPostFx;
        case SignalChain::Stage::Pedal:
            return BlockId::PedalDrive;
        case SignalChain::Stage::Amp:
        default:
            return BlockId::AmpProfiler;
    }
}

int SignalChainStrip::nearestMovableSlot(juce::Point<int> position) const noexcept {
    int nearest = SignalChain::firstMovableSlot;
    auto bestDistance = std::numeric_limits<float>::max();
    for (int slot = SignalChain::firstMovableSlot; slot <= SignalChain::lastMovableSlot; ++slot) {
        const auto centre = _slotBounds[static_cast<size_t>(slot)].getCentre().toFloat();
        const auto distance = centre.getDistanceFrom(position.toFloat());
        if (distance < bestDistance) {
            bestDistance = distance;
            nearest = slot;
        }
    }
    return nearest;
}

int SignalChainStrip::slotAtPosition(juce::Point<int> position) const noexcept {
    for (int slot = SignalChain::firstMovableSlot; slot <= SignalChain::lastMovableSlot; ++slot) {
        if (_slotBounds[static_cast<size_t>(slot)].expanded(10).contains(position)) {
            return slot;
        }
    }
    return -1;
}

void SignalChainStrip::startBlockDrag(BlockId blockId, const juce::MouseEvent& event) {
    _dragBlock = blockId;
    _dragTracking = true;
    _dragActive = false;
    _dropSlot = -1;
    _dragStart = event.getEventRelativeTo(this).getPosition();
    _dragOrigin = getBlock(blockId).getBounds();
}

void SignalChainStrip::updateBlockDrag(const juce::MouseEvent& event) {
    if (!_dragTracking) {
        return;
    }

    const auto position = event.getEventRelativeTo(this).getPosition();
    if (!_dragActive && position.getDistanceFrom(_dragStart) < kDragThresholdPx) {
        return;
    }

    if (!_dragActive) {
        _dragActive = true;
        auto& block = getBlock(_dragBlock);
        block.toFront(false);
        setSelectedBlock(_dragBlock);
    }

    const auto deltaX = position.x - _dragStart.x;
    const auto minX = _slotBounds[static_cast<size_t>(SignalChain::firstMovableSlot)].getX() - 12;
    const auto maxX = _slotBounds[static_cast<size_t>(SignalChain::lastMovableSlot)].getX() + 12;
    auto bounds = _dragOrigin.withX(juce::jlimit(minX, maxX, _dragOrigin.getX() + deltaX));
    getBlock(_dragBlock).setBounds(bounds);
    _dropSlot = nearestMovableSlot(bounds.getCentre());
    repaint();
}

void SignalChainStrip::finishBlockDrag(const juce::MouseEvent&) {
    if (!_dragTracking) {
        return;
    }

    const auto wasDragging = _dragActive;
    const auto dropSlot = _dropSlot;
    const auto blockId = _dragBlock;
    _dragTracking = false;
    _dragActive = false;
    _dropSlot = -1;

    if (wasDragging && dropSlot >= SignalChain::firstMovableSlot && dropSlot <= SignalChain::lastMovableSlot) {
        commitDrop(blockId, dropSlot);
    }

    placeBlocks();
    updateBusLayer();
    _busLayer.toBack();
    if (isBlockOnChain(blockId)) {
        getBlock(blockId).toFront(false);
    }
    repaint();
}

void SignalChainStrip::commitDrop(BlockId blockId, int slot) {
    _layout.place(stageForBlock(blockId), slot);
    if (onLayoutChanged) {
        onLayoutChanged(_layout);
    }
}

void SignalChainStrip::requestPicker(int slot) {
    if (slot < SignalChain::firstMovableSlot || slot > SignalChain::lastMovableSlot) {
        return;
    }

    if (onBlockPickerRequested) {
        onBlockPickerRequested(slot);
    }
}

int SignalChainStrip::fillVisualOrder(std::array<BlockId, 8>& order) const noexcept {
    int count = 0;
    order[static_cast<size_t>(count++)] = BlockId::InputGate;
    for (int slot = SignalChain::firstMovableSlot; slot <= SignalChain::lastMovableSlot; ++slot) {
        const auto stage = _layout.atSlot(slot);
        if (stage != SignalChain::Stage::Empty) {
            order[static_cast<size_t>(count++)] = blockForStage(stage);
        }
    }
    order[static_cast<size_t>(count++)] = BlockId::MasterVolume;
    return count;
}
