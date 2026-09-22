#include "Components/BlockPickerPage.h"

#include <vector>

#include "Stylesheet.h"

namespace {
struct CategorySpec {
    BlockPickerPage::Category category;
    const char* title;
    CustomLookAndFeel::RigIcon icon;
    juce::Colour colour;
    SignalChain::Stage stage;
    const char* itemTitle;
};

const CategorySpec kCategories[] = {
    {BlockPickerPage::Category::Amp, "Ampli", CustomLookAndFeel::RigIcon::AmpHead, ProfilerStyle::Colors::rigAmp,
     SignalChain::Stage::Amp, "Amp"},
    {BlockPickerPage::Category::Cab, "Cab", CustomLookAndFeel::RigIcon::Cabinet, ProfilerStyle::Colors::rigCab,
     SignalChain::Stage::Cab, "Cab"},
    {BlockPickerPage::Category::Eq, "EQ", CustomLookAndFeel::RigIcon::EqFaders, ProfilerStyle::Colors::rigEq,
     SignalChain::Stage::Eq, "EQ"},
    {BlockPickerPage::Category::Pedal, "Pedals", CustomLookAndFeel::RigIcon::Pedal, ProfilerStyle::Colors::rigPedal,
     SignalChain::Stage::Pedal, "Overdrive"}};

const CategorySpec& specFor(BlockPickerPage::Category category) {
    for (const auto& spec : kCategories) {
        if (spec.category == category) {
            return spec;
        }
    }
    return kCategories[0];
}
}  // namespace

BlockPickerPage::TileButton::TileButton()
    : juce::Button({}) {
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void BlockPickerPage::TileButton::setTile(const juce::String& title,
                                          CustomLookAndFeel::RigIcon icon,
                                          juce::Colour colour,
                                          bool inUse) {
    _title = title;
    _icon = icon;
    _colour = colour;
    _inUse = inUse;
    repaint();
}

void BlockPickerPage::TileButton::paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) {
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);
    auto* laf = dynamic_cast<CustomLookAndFeel*>(&getLookAndFeel());
    const auto fill = juce::Colours::black;
    const auto border = _colour.withAlpha(isButtonDown ? 1.0f : (isMouseOverButton ? 0.95f : 0.72f));

    g.setColour(fill);
    g.fillRoundedRectangle(bounds, 8.0f);
    g.setColour(border);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 8.0f, isMouseOverButton || getToggleState() ? 2.0f : 1.0f);

    auto content = bounds.reduced(10.0f, 8.0f);
    const auto iconSize = juce::jmin(42.0f, content.getHeight() * 0.46f);
    auto iconBounds = content.removeFromTop(iconSize + 6.0f).withSizeKeepingCentre(iconSize, iconSize);
    if (laf != nullptr) {
        laf->drawRigIcon(g, iconBounds, _icon, _colour);
    }

    g.setColour(ProfilerStyle::Colors::text);
    g.setFont(ProfilerStyle::Fonts::medium(13.0f));
    g.drawFittedText(_title, content.toNearestInt(), juce::Justification::centred, 1);

    if (_inUse) {
        g.setColour(_colour.withAlpha(0.9f));
        g.fillEllipse(bounds.getRight() - 14.0f, bounds.getY() + 8.0f, 6.0f, 6.0f);
    }
}

BlockPickerPage::BlockPickerPage() {
    _title.setText("CHOOSE A BLOCK", juce::dontSendNotification);
    _title.setFont(ProfilerStyle::Fonts::moduleTitle());
    _title.setColour(juce::Label::textColourId, ProfilerStyle::Colors::text);
    _title.setJustificationType(juce::Justification::centredLeft);
    _title.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(_title);
    addAndMakeVisible(_backButton);
    addAndMakeVisible(_closeButton);
    addAndMakeVisible(_ampCategory);
    addAndMakeVisible(_cabCategory);
    addAndMakeVisible(_eqCategory);
    addAndMakeVisible(_pedalCategory);
    addChildComponent(_itemButton);

    _backButton.setColour(juce::TextButton::buttonColourId, juce::Colours::black);
    _closeButton.setColour(juce::TextButton::buttonColourId, juce::Colours::black);

    _ampCategory.onClick = [this]() {
        showItems(Category::Amp);
    };
    _cabCategory.onClick = [this]() {
        showItems(Category::Cab);
    };
    _eqCategory.onClick = [this]() {
        showItems(Category::Eq);
    };
    _pedalCategory.onClick = [this]() {
        showItems(Category::Pedal);
    };
    _itemButton.onClick = [this]() {
        chooseStage(specFor(_category).stage);
    };
    _backButton.onClick = [this]() {
        showCategories();
    };
    _closeButton.onClick = [this]() {
        close();
    };

    setOpaque(true);
    setVisible(false);
}

void BlockPickerPage::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::black);
    g.setColour(ProfilerStyle::Colors::border);
    g.drawRect(getLocalBounds(), 1);
}

void BlockPickerPage::resized() {
    auto area = getLocalBounds().reduced(12, 8);
    auto header = area.removeFromTop(28);
    _closeButton.setBounds(header.removeFromRight(72).withSizeKeepingCentre(72, 24));
    header.removeFromRight(8);
    _backButton.setBounds(header.removeFromRight(72).withSizeKeepingCentre(72, 24));
    header.removeFromRight(10);
    _title.setBounds(header);
    area.removeFromTop(10);

    if (_showingItems) {
        _ampCategory.setVisible(false);
        _cabCategory.setVisible(false);
        _eqCategory.setVisible(false);
        _pedalCategory.setVisible(false);
        _itemButton.setVisible(true);
        layoutTiles(area, {&_itemButton});
        return;
    }

    _itemButton.setVisible(false);
    _ampCategory.setVisible(true);
    _cabCategory.setVisible(true);
    _eqCategory.setVisible(true);
    _pedalCategory.setVisible(true);
    layoutTiles(area, {&_ampCategory, &_cabCategory, &_eqCategory, &_pedalCategory});
}

void BlockPickerPage::open(int chainSlot, const SignalChain::Layout& layout) {
    _targetSlot = chainSlot;
    _layout = layout;
    showCategories();
    setVisible(true);
    toFront(true);
}

void BlockPickerPage::close() {
    setVisible(false);
    if (onClosed) {
        onClosed();
    }
}

void BlockPickerPage::showCategories() {
    _showingItems = false;
    _backButton.setEnabled(false);
    _title.setText("CHOOSE A BLOCK", juce::dontSendNotification);

    const CategorySpec* specs[] = {&kCategories[0], &kCategories[1], &kCategories[2], &kCategories[3]};
    TileButton* tiles[] = {&_ampCategory, &_cabCategory, &_eqCategory, &_pedalCategory};
    for (int index = 0; index < 4; ++index) {
        tiles[index]->setTile(specs[index]->title,
                              specs[index]->icon,
                              specs[index]->colour,
                              _layout.contains(specs[index]->stage));
    }
    resized();
}

void BlockPickerPage::showItems(Category category) {
    _category = category;
    _showingItems = true;
    _backButton.setEnabled(true);
    const auto& spec = specFor(category);
    _title.setText(juce::String(spec.title).toUpperCase(), juce::dontSendNotification);
    _itemButton.setTile(spec.itemTitle, spec.icon, spec.colour, _layout.contains(spec.stage));
    resized();
}

void BlockPickerPage::chooseStage(SignalChain::Stage stage) {
    const auto slot = _targetSlot;
    if (onBlockChosen) {
        onBlockChosen(slot, stage);
    }
    close();
}

void BlockPickerPage::layoutTiles(juce::Rectangle<int> area, const std::vector<TileButton*>& tiles) {
    if (tiles.empty() || area.isEmpty()) {
        return;
    }

    const auto count = static_cast<int>(tiles.size());
    const auto gap = 10;
    const auto tileW = juce::jlimit(96, 168, (area.getWidth() - gap * (count - 1)) / count);
    const auto tileH = juce::jlimit(88, 120, area.getHeight());
    auto row = juce::Rectangle<int>(tileW * count + gap * (count - 1), tileH).withCentre(area.getCentre());
    for (int index = 0; index < count; ++index) {
        tiles[static_cast<size_t>(index)]->setBounds(row.removeFromLeft(tileW));
        if (index + 1 < count) {
            row.removeFromLeft(gap);
        }
    }
}
