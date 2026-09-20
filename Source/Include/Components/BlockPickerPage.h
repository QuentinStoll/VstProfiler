#pragma once

#include <JuceHeader.h>

#include "Components/CustomLookAndFeel.h"
#include "SignalChainLayout.h"

class BlockPickerPage : public juce::Component {
   public:
    BlockPickerPage();
    ~BlockPickerPage() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void open(int chainSlot, const SignalChain::Layout& layout);
    void close();
    bool isOpen() const noexcept { return isVisible(); }
    int targetSlot() const noexcept { return _targetSlot; }

    enum class Category {
        Amp,
        Cab,
        Eq,
        Pedal
    };

    std::function<void(int slot, SignalChain::Stage stage)> onBlockChosen;
    std::function<void()> onClosed;

   private:
    class TileButton : public juce::Button {
       public:
        TileButton();
        void setTile(const juce::String& title,
                     CustomLookAndFeel::RigIcon icon,
                     juce::Colour colour,
                     bool inUse);
        void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override;

       private:
        juce::String _title;
        CustomLookAndFeel::RigIcon _icon = CustomLookAndFeel::RigIcon::AmpHead;
        juce::Colour _colour = juce::Colours::white;
        bool _inUse = false;
    };

    juce::Label _title;
    juce::TextButton _backButton{"Back"};
    juce::TextButton _closeButton{"Close"};
    TileButton _ampCategory;
    TileButton _cabCategory;
    TileButton _eqCategory;
    TileButton _pedalCategory;
    TileButton _itemButton;
    int _targetSlot = SignalChain::firstMovableSlot;
    SignalChain::Layout _layout{};
    bool _showingItems = false;
    Category _category = Category::Amp;

    void showCategories();
    void showItems(Category category);
    void chooseStage(SignalChain::Stage stage);
    void layoutTiles(juce::Rectangle<int> area, const std::vector<TileButton*>& tiles);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BlockPickerPage)
};
