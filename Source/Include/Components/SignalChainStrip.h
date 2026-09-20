#pragma once

#include <array>

#include <JuceHeader.h>

#include "Components/CustomLookAndFeel.h"
#include "Stylesheet.h"

class SignalChainBlock : public juce::Button {
   public:
    SignalChainBlock(const juce::String& title,
                     const juce::String& subtitle,
                     juce::Colour categoryColour,
                     CustomLookAndFeel::RigIcon icon);
    ~SignalChainBlock() override = default;

    void setSubtitle(const juce::String& subtitle);
    void setLedOn(bool shouldBeOn);
    void setIoNode(bool isIoNode);
    bool isIoNode() const noexcept { return _isIoNode; }

   protected:
    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override;
    bool hitTest(int x, int y) override;

   private:
    juce::String _title;
    juce::String _subtitle;
    juce::Colour _categoryColour;
    CustomLookAndFeel::RigIcon _icon;
    bool _ledOn = false;
    bool _isIoNode = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SignalChainBlock)
};

class SignalChainStrip : public juce::Component {
   public:
    enum class BlockId {
        InputGate = 0,
        AmpProfiler,
        CabinetIr,
        EqPostFx,
        MasterVolume
    };

    SignalChainStrip();
    ~SignalChainStrip() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setSelectedBlock(BlockId blockId);
    BlockId getSelectedBlock() const noexcept { return _selected; }

    void setBlockSubtitle(BlockId blockId, const juce::String& subtitle);
    void setBlockLed(BlockId blockId, bool isOn);

    std::function<void(BlockId)> onBlockSelected;

   private:
    static constexpr int kSlotCount = 8;
    static constexpr int kOccupiedSlots[5] = {0, 2, 4, 6, 7};

    SignalChainBlock _inputGate{"Input / Gate", "Gate", ProfilerStyle::Colors::rigInput,
                                CustomLookAndFeel::RigIcon::InputJack};
    SignalChainBlock _ampProfiler{"Amp Profiler", "Load", ProfilerStyle::Colors::rigAmp,
                                  CustomLookAndFeel::RigIcon::AmpHead};
    SignalChainBlock _cabinetIr{"Cabinet / IR", "Load IR", ProfilerStyle::Colors::rigCab,
                                CustomLookAndFeel::RigIcon::Cabinet};
    SignalChainBlock _eqPostFx{"EQ / Post-FX", "Bypassed", ProfilerStyle::Colors::rigEq,
                               CustomLookAndFeel::RigIcon::EqFaders};
    SignalChainBlock _masterVolume{"Master Volume", "Vol", ProfilerStyle::Colors::rigMaster,
                                   CustomLookAndFeel::RigIcon::Speaker};
    std::array<juce::Rectangle<int>, kSlotCount> _slotBounds{};
    BlockId _selected = BlockId::AmpProfiler;

    SignalChainBlock& getBlock(BlockId blockId);
    const SignalChainBlock& getBlock(BlockId blockId) const;
    void handleBlockClick(BlockId blockId);
    bool isOccupiedSlot(int slot) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SignalChainStrip)
};
