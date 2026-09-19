#pragma once

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

   protected:
    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override;

   private:
    juce::String _title;
    juce::String _subtitle;
    juce::Colour _categoryColour;
    CustomLookAndFeel::RigIcon _icon;
    bool _ledOn = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SignalChainBlock)
};

class SignalChainStrip : public juce::Component {
   public:
    enum class BlockId {
        InputGate = 0,
        AmpProfiler,
        CabinetIr,
        EqPostFx
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
    SignalChainBlock _inputGate{"Input / Gate", "Noise Gate", ProfilerStyle::Colors::rigInput,
                                CustomLookAndFeel::RigIcon::InputJack};
    SignalChainBlock _ampProfiler{"Amp Profiler", "Load NAM", ProfilerStyle::Colors::rigAmp,
                                  CustomLookAndFeel::RigIcon::AmpHead};
    SignalChainBlock _cabinetIr{"Cabinet / IR", "Load IR", ProfilerStyle::Colors::rigCab,
                                CustomLookAndFeel::RigIcon::Cabinet};
    SignalChainBlock _eqPostFx{"EQ / Post-FX", "Tone stack", ProfilerStyle::Colors::rigEq,
                               CustomLookAndFeel::RigIcon::EqFaders};
    BlockId _selected = BlockId::AmpProfiler;

    SignalChainBlock& getBlock(BlockId blockId);
    const SignalChainBlock& getBlock(BlockId blockId) const;
    void handleBlockClick(BlockId blockId);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SignalChainStrip)
};
