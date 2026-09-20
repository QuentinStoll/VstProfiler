#pragma once

#include <array>

#include <JuceHeader.h>

#include "Components/CustomLookAndFeel.h"
#include "Stylesheet.h"

class SignalChainBlock : public juce::Button {
   public:
    SignalChainBlock(juce::Colour categoryColour, CustomLookAndFeel::RigIcon icon);
    ~SignalChainBlock() override = default;

    void setLedOn(bool shouldBeOn);
    void setIoNode(bool isIoNode);
    void setShowsLed(bool shouldShowLed);
    void setSignalLevel(float level);
    bool isIoNode() const noexcept { return _isIoNode; }

    std::function<void()> onLedClicked;

   protected:
    void paint(juce::Graphics& g) override;
    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override;
    bool hitTest(int x, int y) override;
    void mouseUp(const juce::MouseEvent& event) override;

   private:
    juce::Colour _categoryColour;
    CustomLookAndFeel::RigIcon _icon;
    bool _ledOn = true;
    bool _isIoNode = false;
    bool _showsLed = true;
    float _signalLevel = 0.0f;

    juce::Rectangle<float> getLedBounds() const;
    bool isLedHit(juce::Point<int> position) const;
    void paintContents(juce::Graphics& g, bool isMouseOverButton);

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
    bool selectAdjacentBlock(int delta);

    bool keyPressed(const juce::KeyPress& key) override;

    void setBlockLed(BlockId blockId, bool isOn);
    void setIoMeterLevels(float inputDb, float outputDb);

    std::function<void(BlockId)> onBlockSelected;
    std::function<void(BlockId)> onBlockBypassToggled;

   private:
    class SignalBusLayer : public juce::Component {
       public:
        void setLine(float y, float x1, float x2);
        void paint(juce::Graphics& g) override;

       private:
        float _y = 0.0f;
        float _x1 = 0.0f;
        float _x2 = 0.0f;
    };

    static constexpr int kSlotCount = 8;
    static constexpr int kOccupiedSlots[5] = {0, 2, 4, 6, 7};

    SignalBusLayer _busLayer;
    SignalChainBlock _inputGate{ProfilerStyle::Colors::rigInput, CustomLookAndFeel::RigIcon::InputJack};
    SignalChainBlock _ampProfiler{ProfilerStyle::Colors::rigAmp, CustomLookAndFeel::RigIcon::AmpHead};
    SignalChainBlock _cabinetIr{ProfilerStyle::Colors::rigCab, CustomLookAndFeel::RigIcon::Cabinet};
    SignalChainBlock _eqPostFx{ProfilerStyle::Colors::rigEq, CustomLookAndFeel::RigIcon::EqFaders};
    SignalChainBlock _masterVolume{ProfilerStyle::Colors::rigMaster, CustomLookAndFeel::RigIcon::Speaker};
    std::array<juce::Rectangle<int>, kSlotCount> _slotBounds{};
    BlockId _selected = BlockId::AmpProfiler;
    float _inputMeter = 0.0f;
    float _outputMeter = 0.0f;
    double _lastMeterMs = 0.0;

    SignalChainBlock& getBlock(BlockId blockId);
    const SignalChainBlock& getBlock(BlockId blockId) const;
    void handleBlockClick(BlockId blockId);
    bool isOccupiedSlot(int slot) const;
    void updateBusLayer();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SignalChainStrip)
};
