#include "Components/CustomLookAndFeel.h"

#include "Stylesheet.h"

namespace {
constexpr float kButtonCorner = ProfilerStyle::Surfaces::controlCorner;
constexpr float kPanelCorner = ProfilerStyle::Surfaces::panelCorner;

bool getPropertyFlag(const juce::Component& component,
                     const juce::Identifier& propertyId,
                     bool defaultValue) {
    return static_cast<bool>(component.getProperties().getWithDefault(propertyId, defaultValue));
}

juce::Colour getStoredColour(const juce::Component& component,
                             const juce::Identifier& propertyId,
                             juce::Colour fallback) {
    if (auto* value = component.getProperties().getVarPointer(propertyId)) {
        return juce::Colour(static_cast<juce::uint32>(static_cast<int>(*value)));
    }

    return fallback;
}
}  // namespace

CustomLookAndFeel::CustomLookAndFeel()
    : _typefaceName(ProfilerStyle::Fonts::defaultFamily()) {
    setDefaultSansSerifTypefaceName(_typefaceName);
    applyColourScheme();
}

void CustomLookAndFeel::applyColourScheme() {
    using namespace ProfilerStyle::Colors;

    setColourScheme({background, container, container, border, text, accent, text, accent, text});

    setColour(juce::ResizableWindow::backgroundColourId, background);
    setColour(juce::DocumentWindow::backgroundColourId, background);

    setColour(juce::TextButton::buttonColourId, elevated);
    setColour(juce::TextButton::buttonOnColourId, accent);
    setColour(juce::TextButton::textColourOffId, text);
    setColour(juce::TextButton::textColourOnId, text);

    setColour(juce::ToggleButton::textColourId, text);
    setColour(juce::ToggleButton::tickColourId, accent);
    setColour(juce::ToggleButton::tickDisabledColourId, textMuted);

    setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::Label::textColourId, text);
    setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);

    setColour(juce::Slider::backgroundColourId, border);
    setColour(juce::Slider::thumbColourId, accent);
    setColour(juce::Slider::trackColourId, accent);
    setColour(juce::Slider::rotarySliderFillColourId, accent);
    setColour(juce::Slider::rotarySliderOutlineColourId, border);
    setColour(juce::Slider::textBoxTextColourId, text);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);

    setColour(juce::ComboBox::backgroundColourId, elevated);
    setColour(juce::ComboBox::textColourId, text);
    setColour(juce::ComboBox::outlineColourId, border);
    setColour(juce::ComboBox::arrowColourId, textMuted);
    setColour(juce::ComboBox::focusedOutlineColourId, accent);

    setColour(juce::PopupMenu::backgroundColourId, container);
    setColour(juce::PopupMenu::textColourId, text);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, accent.withAlpha(0.18f));
    setColour(juce::PopupMenu::highlightedTextColourId, text);

    setColour(juce::TextEditor::backgroundColourId, elevated);
    setColour(juce::TextEditor::textColourId, text);
    setColour(juce::TextEditor::outlineColourId, border);
    setColour(juce::TextEditor::focusedOutlineColourId, accent);
    setColour(juce::TextEditor::highlightColourId, accent.withAlpha(0.28f));
    setColour(juce::TextEditor::highlightedTextColourId, text);
    setColour(juce::CaretComponent::caretColourId, accent);

    setColour(juce::ScrollBar::thumbColourId, accent);
    setColour(juce::ScrollBar::trackColourId, container);
}

juce::Font CustomLookAndFeel::getUiFont(float height, juce::Font::FontStyleFlags style) const {
    return juce::Font(juce::FontOptions(_typefaceName, height, style));
}

juce::Typeface::Ptr CustomLookAndFeel::getTypefaceForFont(const juce::Font& font) {
    auto named = font;
    named.setTypefaceName(_typefaceName);
    return juce::LookAndFeel_V4::getTypefaceForFont(named);
}

juce::Font CustomLookAndFeel::getLabelFont(juce::Label& label) {
    const auto height = label.getFont().getHeight() > 0.0f ? label.getFont().getHeight() : 14.0f;
    return getUiFont(height);
}

juce::Font CustomLookAndFeel::getTextButtonFont(juce::TextButton&, int buttonHeight) {
    return getUiFont(juce::jlimit(11.0f, 15.0f, static_cast<float>(buttonHeight) * 0.38f));
}

juce::Font CustomLookAndFeel::getComboBoxFont(juce::ComboBox& box) {
    return getUiFont(juce::jlimit(12.0f, 16.0f, static_cast<float>(box.getHeight()) * 0.42f));
}

juce::Font CustomLookAndFeel::getPopupMenuFont() {
    return getUiFont(14.0f);
}

juce::Font CustomLookAndFeel::getSliderPopupFont(juce::Slider&) {
    return getUiFont(13.0f);
}

void CustomLookAndFeel::drawAccentGlow(juce::Graphics& g, juce::Rectangle<float> bounds, float intensity,
                                       juce::Colour colour) const {
    if (intensity <= 0.0f) {
        return;
    }

    if (colour.isTransparent()) {
        colour = ProfilerStyle::Colors::accent;
    }

    juce::DropShadow(colour.withAlpha(0.38f * intensity),
                     juce::roundToInt(8.0f + 6.0f * intensity),
                     {})
        .drawForRectangle(g, bounds.toNearestInt());
}

juce::String CustomLookAndFeel::ellipsize(const juce::Font& font, const juce::String& text, float maxWidth) const {
    if (maxWidth <= 0.0f || text.isEmpty() || juce::GlyphArrangement::getStringWidth(font, text) <= maxWidth) {
        return text;
    }

    const auto ellipsis = juce::String("...");
    auto trimmed = text;

    while (trimmed.isNotEmpty() && juce::GlyphArrangement::getStringWidth(font, trimmed + ellipsis) > maxWidth) {
        trimmed = trimmed.dropLastCharacters(1);
    }

    return trimmed.isEmpty() ? ellipsis : trimmed + ellipsis;
}

void CustomLookAndFeel::drawRaisedPanel(juce::Graphics& g,
                                        juce::Rectangle<float> bounds,
                                        bool glow,
                                        float cornerRadius) const {
    if (glow) {
        drawAccentGlow(g, bounds.reduced(2.0f), 0.7f);
    }

    g.setColour(ProfilerStyle::Colors::elevated);
    g.fillRoundedRectangle(bounds, cornerRadius);
    g.setColour(glow ? ProfilerStyle::Colors::accent.withAlpha(0.85f) : ProfilerStyle::Colors::border);
    g.drawRoundedRectangle(bounds.reduced(0.5f), cornerRadius, 1.0f);
}

void CustomLookAndFeel::paintFlatButtonBackground(juce::Graphics& g,
                                                  juce::Rectangle<float> bounds,
                                                  juce::Colour backgroundColour,
                                                  bool isMouseOver,
                                                  bool isButtonDown,
                                                  bool isOn,
                                                  bool outlineVisible,
                                                  juce::Colour outlineColour) const {
    auto fillBounds = outlineVisible ? bounds.reduced(1.5f) : bounds;
    auto fill = backgroundColour;

    if (isButtonDown) {
        fill = fill.darker(0.12f);
    } else if (isMouseOver) {
        fill = fill.brighter(0.08f);
    }

    const auto glowIntensity = isButtonDown ? 0.85f : (isOn ? 0.7f : (isMouseOver ? 0.38f : 0.0f));
    drawAccentGlow(g, fillBounds.reduced(1.0f), glowIntensity);

    g.setColour(fill);
    g.fillRoundedRectangle(fillBounds, kButtonCorner);

    if (glowIntensity > 0.0f) {
        g.setColour(ProfilerStyle::Colors::accent.withAlpha(0.1f * glowIntensity));
        g.fillRoundedRectangle(fillBounds, kButtonCorner);
        g.setColour(ProfilerStyle::Colors::accent.withAlpha(0.55f * glowIntensity));
        g.drawRoundedRectangle(fillBounds.reduced(0.5f), kButtonCorner, 1.0f);
    } else {
        g.setColour(ProfilerStyle::Colors::border);
        g.drawRoundedRectangle(fillBounds.reduced(0.5f), kButtonCorner, 1.0f);
    }

    if (outlineVisible) {
        const auto colour = outlineColour.isTransparent() ? ProfilerStyle::Colors::lightestGrey
                                                          : outlineColour;
        g.setColour(colour.withAlpha(0.45f));
        g.drawRoundedRectangle(bounds.reduced(0.5f), kButtonCorner + 1.0f, 1.0f);
    }
}

void CustomLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                             const juce::Colour& backgroundColour,
                                             bool isMouseOverButton, bool isButtonDown) {
    paintFlatButtonBackground(g,
                              button.getLocalBounds().toFloat(),
                              backgroundColour,
                              isMouseOverButton,
                              isButtonDown,
                              button.getToggleState(),
                              getPropertyFlag(button, ProfilerStyle::Properties::outlineVisible, false),
                              getStoredColour(button, ProfilerStyle::Properties::outlineColour,
                                              ProfilerStyle::Colors::lightestGrey));
}

void CustomLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button,
                                       bool /*isMouseOverButton*/, bool /*isButtonDown*/) {
    const auto alpha = button.isEnabled() ? 1.0f : 0.45f;
    g.setColour(button.findColour(button.getToggleState() ? juce::TextButton::textColourOnId
                                                          : juce::TextButton::textColourOffId)
                    .withMultipliedAlpha(alpha));
    g.setFont(getTextButtonFont(button, button.getHeight()));
    g.drawText(button.getButtonText(), button.getLocalBounds(), juce::Justification::centred, true);
}

void CustomLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPos, const float rotaryStartAngle,
                                         const float rotaryEndAngle, juce::Slider& slider) {
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(3.0f);
    const auto side = std::min(bounds.getWidth(), bounds.getHeight());
    auto knobArea = bounds.withSizeKeepingCentre(side, side);
    const auto centre = knobArea.getCentre();
    const auto radius = side * 0.5f;
    const auto enabledAlpha = slider.isEnabled() ? 1.0f : 0.35f;
    const auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const auto lineW = juce::jlimit(1.5f, 2.4f, radius * 0.08f);
    const auto arcRadius = radius - lineW * 1.2f;
    const auto isActive = slider.isEnabled() && (slider.isMouseOverOrDragging() || slider.hasKeyboardFocus(true));
    const auto accent = slider.findColour(juce::Slider::rotarySliderFillColourId);

    juce::Path backgroundArc;
    backgroundArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                                rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(ProfilerStyle::Colors::border.withAlpha(enabledAlpha));
    g.strokePath(backgroundArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    if (slider.isEnabled()) {
        juce::Path valueArc;
        valueArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                               rotaryStartAngle, toAngle, true);

        if (isActive) {
            g.setColour(accent.withAlpha(0.22f));
            g.strokePath(valueArc, juce::PathStrokeType(lineW * 2.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        g.setColour(accent.withAlpha(enabledAlpha));
        g.strokePath(valueArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    if (slider.getTextBoxPosition() == juce::Slider::NoTextBox) {
        const auto valueText = slider.getTextFromValue(slider.getValue()).trim();
        const auto fontHeight = juce::jlimit(8.0f, 11.0f, radius * 0.32f);
        g.setFont(getUiFont(fontHeight, juce::Font::bold));
        g.setColour(ProfilerStyle::Colors::text.withAlpha(enabledAlpha));
        g.drawFittedText(valueText,
                         knobArea.reduced(radius * 0.28f).toNearestInt(),
                         juce::Justification::centred,
                         2);
    }
}

void CustomLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPos, float minSliderPos, float maxSliderPos,
                                         const juce::Slider::SliderStyle style, juce::Slider& slider) {
    juce::ignoreUnused(minSliderPos, maxSliderPos);

    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(8.0f);
    if (bounds.isEmpty()) {
        return;
    }

    const auto isVertical = style == juce::Slider::LinearVertical || style == juce::Slider::LinearBarVertical;
    const auto enabledAlpha = slider.isEnabled() ? 1.0f : 0.35f;
    const auto trackThickness = 4.0f;
    const auto isActive = slider.isEnabled() && (slider.isMouseOverOrDragging() || slider.hasKeyboardFocus(true));

    juce::Rectangle<float> trackBounds = isVertical
                                             ? bounds.withSizeKeepingCentre(trackThickness, bounds.getHeight())
                                             : bounds.withSizeKeepingCentre(bounds.getWidth(), trackThickness);

    g.setColour(ProfilerStyle::Colors::border.withAlpha(enabledAlpha));
    g.fillRoundedRectangle(trackBounds, trackThickness * 0.5f);

    const auto clampedSliderPos = isVertical
                                      ? juce::jlimit(trackBounds.getY(), trackBounds.getBottom(), sliderPos)
                                      : juce::jlimit(trackBounds.getX(), trackBounds.getRight(), sliderPos);

    if (slider.isEnabled()) {
        juce::Rectangle<float> valueBounds = isVertical
                                                 ? juce::Rectangle<float>(trackBounds.getX(), clampedSliderPos,
                                                                          trackBounds.getWidth(), trackBounds.getBottom() - clampedSliderPos)
                                                 : juce::Rectangle<float>(trackBounds.getX(), trackBounds.getY(),
                                                                          clampedSliderPos - trackBounds.getX(), trackBounds.getHeight());

        if (!valueBounds.isEmpty()) {
            g.setColour(ProfilerStyle::Colors::accent.withAlpha(isActive ? 0.22f : 0.12f));
            g.fillRoundedRectangle(valueBounds.expanded(isVertical ? 2.0f : 0.0f, isVertical ? 0.0f : 2.0f),
                                   trackThickness * 0.5f);
            g.setColour(ProfilerStyle::Colors::accent);
            g.fillRoundedRectangle(valueBounds, trackThickness * 0.5f);
        }
    }

    const auto thumbSize = 12.0f;
    juce::Rectangle<float> thumbBounds(
        isVertical ? bounds.getCentreX() - thumbSize * 0.5f : clampedSliderPos - thumbSize * 0.5f,
        isVertical ? clampedSliderPos - thumbSize * 0.5f : bounds.getCentreY() - thumbSize * 0.5f,
        thumbSize, thumbSize);

    if (slider.isEnabled() && isActive) {
        juce::DropShadow(ProfilerStyle::Colors::accent.withAlpha(0.4f), 8, {})
            .drawForRectangle(g, thumbBounds.toNearestInt());
    }

    g.setColour(ProfilerStyle::Colors::elevated.withAlpha(enabledAlpha));
    g.fillEllipse(thumbBounds);
    g.setColour(ProfilerStyle::Colors::accent.withAlpha(enabledAlpha));
    g.drawEllipse(thumbBounds.reduced(0.5f), 1.4f);
}

void CustomLookAndFeel::drawSwitch(juce::Graphics& g, juce::Rectangle<float> area,
                                   bool ticked, bool isEnabled, bool isMouseOver, bool isMouseDown) const {
    area = area.reduced(1.0f);
    if (area.getWidth() <= 4.0f || area.getHeight() <= 4.0f) {
        return;
    }

    const auto enabledAlpha = isEnabled ? 1.0f : 0.35f;
    const auto trackH = juce::jmin(area.getHeight() * 0.7f, 22.0f);
    const auto trackW = juce::jmin(area.getWidth() * 0.86f, trackH * 1.9f);
    auto trackRect = area.withSizeKeepingCentre(trackW, trackH);
    const auto cornerSize = trackH * 0.5f;
    auto trackColour = ticked ? ProfilerStyle::Colors::accent : ProfilerStyle::Colors::border;

    if (isMouseOver) {
        trackColour = ticked ? trackColour.brighter(0.08f) : trackColour.brighter(0.06f);
    }
    if (isMouseDown) {
        trackColour = trackColour.darker(0.08f);
    }

    if (ticked && isEnabled) {
        drawAccentGlow(g, trackRect, isMouseOver ? 0.7f : 0.5f);
    }

    g.setColour(trackColour.withAlpha(enabledAlpha * (ticked ? 0.85f : 1.0f)));
    g.fillRoundedRectangle(trackRect, cornerSize);

    g.setColour((ticked ? ProfilerStyle::Colors::accent : ProfilerStyle::Colors::lightestGrey)
                    .withAlpha(enabledAlpha * 0.45f));
    g.drawRoundedRectangle(trackRect.reduced(0.5f), cornerSize, 1.0f);

    const auto thumbPadding = 2.0f;
    const auto thumbSize = trackH - thumbPadding * 2.0f;
    const auto thumbX = ticked ? trackRect.getRight() - thumbSize - thumbPadding
                               : trackRect.getX() + thumbPadding;
    auto thumbRect = juce::Rectangle<float>(thumbX, trackRect.getCentreY() - thumbSize * 0.5f, thumbSize, thumbSize);

    g.setColour(ProfilerStyle::Colors::text.withAlpha(enabledAlpha));
    g.fillEllipse(thumbRect);
}

void CustomLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                         bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) {
    auto area = button.getLocalBounds().toFloat();

    if (!getPropertyFlag(button, ProfilerStyle::Properties::toggleLabelVisible, true)) {
        drawSwitch(g, area, button.getToggleState(), button.isEnabled(),
                   shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
        return;
    }

    const auto labelHeight = juce::jlimit(8.0f, 20.0f, area.getHeight() * 0.48f);
    auto labelArea = area.removeFromTop(labelHeight);

    g.setColour(button.findColour(juce::ToggleButton::textColourId).withAlpha(button.isEnabled() ? 1.0f : 0.45f));
    g.setFont(getUiFont(juce::jlimit(8.0f, 14.0f, labelHeight * 0.8f)));
    g.drawFittedText(button.getButtonText(), labelArea.toNearestInt(), juce::Justification::centred, 1);

    drawSwitch(g, area, button.getToggleState(), button.isEnabled(),
               shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
}

void CustomLookAndFeel::drawTickBox(juce::Graphics& g, juce::Component& button,
                                    float x, float y, float w, float h,
                                    bool ticked, bool isEnabled, bool isMouseOver, bool isMouseDown) {
    juce::ignoreUnused(button);
    drawSwitch(g, juce::Rectangle<float>(x, y, w, h), ticked, isEnabled, isMouseOver, isMouseDown);
}

void CustomLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                                     int, int, int, int, juce::ComboBox& box) {
    auto area = juce::Rectangle<int>(0, 0, width, height).toFloat().reduced(1.0f);
    paintFlatButtonBackground(g, area, box.findColour(juce::ComboBox::backgroundColourId),
                              box.isMouseOver(), isButtonDown, false);

    auto arrowZone = juce::Rectangle<float>(static_cast<float>(width) - 24.0f, 0.0f, 18.0f, static_cast<float>(height));
    juce::Path path;
    path.addTriangle(arrowZone.getCentreX() - 4.0f, arrowZone.getCentreY() - 2.0f,
                     arrowZone.getCentreX() + 4.0f, arrowZone.getCentreY() - 2.0f,
                     arrowZone.getCentreX(), arrowZone.getCentreY() + 4.0f);

    g.setColour(box.findColour(juce::ComboBox::arrowColourId).withAlpha(box.isEnabled() ? 0.85f : 0.4f));
    g.fillPath(path);
}

void CustomLookAndFeel::positionComboBoxText(juce::ComboBox& box, juce::Label& label) {
    label.setBounds(8, 1, box.getWidth() - 30, box.getHeight() - 2);
    label.setFont(getComboBoxFont(box));
    label.setJustificationType(juce::Justification::centredLeft);
}

void CustomLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height) {
    g.fillAll(ProfilerStyle::Colors::container);
    g.setColour(ProfilerStyle::Colors::border);
    g.drawRect(0, 0, width, height, 1);
}

void CustomLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                                          bool isSeparator, bool isActive,
                                          bool isHighlighted, bool isChecked,
                                          bool /*hasSubMenu*/, const juce::String& text,
                                          const juce::String& /*shortcutKeyText*/,
                                          const juce::Drawable* /*icon*/, const juce::Colour* textColourToUse) {
    if (isSeparator) {
        g.setColour(ProfilerStyle::Colors::border);
        g.fillRect(area.reduced(8, 0).withHeight(1).withY(area.getCentreY()));
        return;
    }

    auto r = area.toFloat().reduced(3.0f, 1.0f);

    if (isHighlighted && isActive) {
        g.setColour(ProfilerStyle::Colors::accent.withAlpha(0.16f));
        g.fillRoundedRectangle(r, 3.0f);
    }

    auto textColour = textColourToUse != nullptr ? *textColourToUse
                                                 : ProfilerStyle::Colors::text.withAlpha(isActive ? 1.0f : 0.45f);
    if (isHighlighted && isActive) {
        textColour = ProfilerStyle::Colors::text;
    }

    g.setColour(textColour);
    g.setFont(getPopupMenuFont());
    g.drawText(text, r.reduced(10.0f, 0), juce::Justification::centredLeft, true);

    if (isChecked) {
        auto indicatorSize = r.getHeight() * 0.28f;
        auto indicatorRect = juce::Rectangle<float>(r.getRight() - 18.0f,
                                                    r.getCentreY() - indicatorSize * 0.5f,
                                                    indicatorSize, indicatorSize);
        g.setColour(ProfilerStyle::Colors::accent);
        g.fillEllipse(indicatorRect);
    }
}

void CustomLookAndFeel::drawTabbedButtonBarBackground(juce::TabbedButtonBar& bar, juce::Graphics& g) {
    g.setColour(ProfilerStyle::Colors::container);
    g.fillRoundedRectangle(bar.getLocalBounds().toFloat(), kPanelCorner);
}

void CustomLookAndFeel::drawTabButton(juce::TabBarButton& button, juce::Graphics& g,
                                      bool isMouseOver, bool isMouseDown) {
    auto area = button.getLocalBounds().toFloat().reduced(2.0f);
    const auto isFront = button.isFrontTab();
    auto fill = isFront ? ProfilerStyle::Colors::elevated : juce::Colours::transparentBlack;

    if (isMouseOver && !isFront) {
        fill = ProfilerStyle::Colors::elevated.withAlpha(0.55f);
    }

    if (isFront) {
        drawAccentGlow(g, area.reduced(2.0f), isMouseDown ? 0.55f : 0.35f);
    }

    if (!fill.isTransparent()) {
        g.setColour(fill);
        g.fillRoundedRectangle(area, kButtonCorner);
    }

    if (isFront) {
        auto indicator = area.removeFromBottom(2.0f).reduced(8.0f, 0.0f);
        g.setColour(ProfilerStyle::Colors::accent.withAlpha(0.7f));
        g.fillRoundedRectangle(indicator, 1.0f);
    }

    g.setColour(isFront ? ProfilerStyle::Colors::text : ProfilerStyle::Colors::textMuted);
    g.setFont(getUiFont(14.0f));
    g.drawText(button.getButtonText(), button.getLocalBounds().toFloat().reduced(2.0f),
               juce::Justification::centred);
}

void CustomLookAndFeel::drawTabAreaBehindFrontButton(juce::TabbedButtonBar&, juce::Graphics&, int, int) {
}

int CustomLookAndFeel::getTabButtonBestWidth(juce::TabBarButton&, int tabDepth) {
    return tabDepth;
}

void CustomLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label) {
    g.fillAll(label.findColour(juce::Label::backgroundColourId));

    if (!label.isBeingEdited()) {
        const auto font = getLabelFont(label);
        g.setColour(label.findColour(juce::Label::textColourId).withMultipliedAlpha(label.isEnabled() ? 1.0f : 0.45f));
        g.setFont(font);
        const auto maxLines = juce::jmax(1, static_cast<int>(label.getHeight() / juce::jmax(1.0f, font.getHeight())));
        g.drawFittedText(label.getText(), label.getLocalBounds(), label.getJustificationType(), maxLines);
    }
}

void CustomLookAndFeel::drawSignalChainBlock(juce::Graphics& g,
                                             juce::Rectangle<float> bounds,
                                             const juce::String& title,
                                             const juce::String& subtitle,
                                             juce::Colour categoryColour,
                                             RigIcon icon,
                                             bool isActive,
                                             bool isMouseOver,
                                             bool ledOn) const {
    if (isActive) {
        drawAccentGlow(g, bounds.reduced(1.0f), 0.72f, categoryColour);
    } else if (isMouseOver) {
        drawAccentGlow(g, bounds.reduced(3.0f), 0.28f, categoryColour);
    }

    g.setColour(ProfilerStyle::Colors::elevated);
    g.fillRoundedRectangle(bounds, kPanelCorner);

    if (isActive) {
        g.setColour(categoryColour.withAlpha(0.1f));
        g.fillRoundedRectangle(bounds, kPanelCorner);
        g.setColour(categoryColour);
        g.drawRoundedRectangle(bounds.reduced(0.5f), kPanelCorner, 1.2f);
    } else {
        g.setColour(ProfilerStyle::Colors::border);
        g.drawRoundedRectangle(bounds.reduced(0.5f), kPanelCorner, 1.0f);
    }

    auto accentBar = juce::Rectangle<float>(bounds.getX() + 10.0f, bounds.getY() + 5.0f,
                                            bounds.getWidth() - 20.0f, 2.0f);
    g.setColour(categoryColour);
    g.fillRoundedRectangle(accentBar, 1.0f);

    auto content = bounds.reduced(10.0f, 14.0f);
    auto led = juce::Rectangle<float>(7.0f, 7.0f).withX(content.getRight() - 7.0f).withY(content.getY() + 2.0f);
    if (ledOn) {
        juce::DropShadow(categoryColour.withAlpha(0.75f), 6, {}).drawForRectangle(g, led.toNearestInt());
        g.setColour(categoryColour);
    } else {
        g.setColour(ProfilerStyle::Colors::border.brighter(0.2f));
    }
    g.fillEllipse(led);

    auto captionBounds = content.removeFromBottom(14.0f);
    content.removeFromBottom(2.0f);
    auto titleBounds = content.removeFromBottom(16.0f);
    content.removeFromBottom(4.0f);
    auto iconBounds = content.withSizeKeepingCentre(juce::jmin(40.0f, content.getWidth()),
                                                    juce::jmin(38.0f, content.getHeight()));
    drawRigIcon(g, iconBounds, icon, categoryColour);

    g.setColour(ProfilerStyle::Colors::text.withAlpha(isActive ? 1.0f : 0.9f));
    g.setFont(getUiFont(11.0f, juce::Font::bold));
    g.drawFittedText(title, titleBounds.toNearestInt(), juce::Justification::centred, 1);

    const auto captionFont = getUiFont(9.0f);
    g.setFont(captionFont);
    g.setColour(ProfilerStyle::Colors::caption);
    g.drawText(ellipsize(captionFont, subtitle, captionBounds.getWidth()),
               captionBounds,
               juce::Justification::centred,
               false);
}

void CustomLookAndFeel::drawRigIcon(juce::Graphics& g, juce::Rectangle<float> bounds, RigIcon icon, juce::Colour colour) const {
    g.setColour(colour);
    const auto stroke = juce::PathStrokeType(1.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);

    switch (icon) {
        case RigIcon::InputJack: {
            auto plate = bounds.withSizeKeepingCentre(bounds.getWidth() * 0.72f, bounds.getHeight() * 0.72f);
            g.drawEllipse(plate, 1.4f);
            auto inner = plate.reduced(plate.getWidth() * 0.22f);
            g.drawEllipse(inner, 1.2f);
            auto slot = inner.withSizeKeepingCentre(inner.getWidth() * 0.22f, inner.getHeight() * 0.55f);
            g.fillRoundedRectangle(slot, 1.0f);
            break;
        }
        case RigIcon::AmpHead: {
            auto head = bounds.reduced(1.5f);
            g.drawRoundedRectangle(head, 2.2f, 1.3f);
            auto controlRow = head.removeFromTop(head.getHeight() * 0.34f).reduced(3.0f, 2.2f);
            const auto knobSize = juce::jmin(4.4f, controlRow.getHeight());
            for (int i = 0; i < 4; ++i) {
                const auto x = controlRow.getX() + (controlRow.getWidth() - knobSize) * (i / 3.0f);
                g.drawEllipse(x, controlRow.getCentreY() - knobSize * 0.5f, knobSize, knobSize, 1.0f);
            }
            auto grille = head.reduced(3.2f, 2.4f);
            const auto dot = 1.5f;
            for (int row = 0; row < 3; ++row) {
                for (int col = 0; col < 5; ++col) {
                    const auto x = grille.getX() + (grille.getWidth() - dot) * (col / 4.0f);
                    const auto y = grille.getY() + (grille.getHeight() - dot) * (row / 2.0f);
                    g.fillEllipse(x, y, dot, dot);
                }
            }
            break;
        }
        case RigIcon::Cabinet: {
            juce::Path cone;
            const auto top = bounds.getY() + bounds.getHeight() * 0.18f;
            const auto bottom = bounds.getBottom() - bounds.getHeight() * 0.12f;
            cone.addQuadrilateral(bounds.getX() + bounds.getWidth() * 0.18f, top,
                                  bounds.getRight() - bounds.getWidth() * 0.18f, top,
                                  bounds.getRight() - bounds.getWidth() * 0.32f, bottom,
                                  bounds.getX() + bounds.getWidth() * 0.32f, bottom);
            g.strokePath(cone, stroke);
            const auto centre = juce::Point<float>(bounds.getCentreX(), top + bounds.getHeight() * 0.22f);
            const auto radius = bounds.getWidth() * 0.16f;
            g.drawEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 1.2f);
            g.fillEllipse(centre.x - radius * 0.35f, centre.y - radius * 0.35f, radius * 0.7f, radius * 0.7f);
            break;
        }
        case RigIcon::EqFaders: {
            const float caps[] = {0.28f, 0.58f, 0.4f};
            const auto trackW = 2.0f;
            const auto capW = 7.0f;
            const auto capH = 3.2f;
            for (int i = 0; i < 3; ++i) {
                const auto x = bounds.getX() + bounds.getWidth() * (0.22f + 0.28f * static_cast<float>(i));
                auto track = juce::Rectangle<float>(x - trackW * 0.5f, bounds.getY() + 2.0f, trackW, bounds.getHeight() - 4.0f);
                g.fillRoundedRectangle(track, 1.0f);
                auto cap = juce::Rectangle<float>(x - capW * 0.5f,
                                                 bounds.getY() + bounds.getHeight() * caps[i] - capH * 0.5f,
                                                 capW, capH);
                g.fillRoundedRectangle(cap, 1.0f);
            }
            break;
        }
    }
}

void CustomLookAndFeel::drawSignalCable(juce::Graphics& g,
                                        juce::Point<float> from,
                                        juce::Point<float> to,
                                        bool isEnergized) const {
    const auto colour = isEnergized ? juce::Colour(0xffC5C5CE) : ProfilerStyle::Colors::border.brighter(0.2f);

    g.setColour(colour.withAlpha(isEnergized ? 0.18f : 0.3f));
    g.drawLine(from.x, from.y, to.x, to.y, 4.0f);
    g.setColour(colour.withAlpha(isEnergized ? 0.9f : 0.65f));
    g.drawLine(from.x, from.y, to.x, to.y, 1.2f);

    const auto node = 4.0f;
    g.setColour(colour);
    g.fillEllipse(from.x - node * 0.5f, from.y - node * 0.5f, node, node);
    g.fillEllipse(to.x - node * 0.5f, to.y - node * 0.5f, node, node);
}

void CustomLookAndFeel::drawDropZone(juce::Graphics& g,
                                     juce::Rectangle<float> bounds,
                                     const juce::String& title,
                                     const juce::String& detail,
                                     bool isLoaded,
                                     bool isDragOver,
                                     RigIcon icon,
                                     juce::Colour categoryColour) const {
    if (isDragOver) {
        drawAccentGlow(g, bounds.reduced(2.0f), 0.8f, categoryColour);
    }

    g.setColour(ProfilerStyle::Colors::background.brighter(isDragOver ? 0.06f : 0.02f));
    g.fillRoundedRectangle(bounds, kPanelCorner);

    g.setColour(isDragOver ? categoryColour
                           : (isLoaded ? categoryColour.withAlpha(0.7f) : ProfilerStyle::Colors::border));
    g.drawRoundedRectangle(bounds.reduced(0.5f), kPanelCorner, isDragOver || isLoaded ? 1.3f : 1.0f);

    auto dashBounds = bounds.reduced(10.0f);
    g.setColour((isDragOver ? categoryColour : ProfilerStyle::Colors::border).withAlpha(0.5f));
    const float dash[] = {5.0f, 4.0f};
    g.drawDashedLine({dashBounds.getX(), dashBounds.getY(), dashBounds.getRight(), dashBounds.getY()}, dash, 2, 1.0f);
    g.drawDashedLine({dashBounds.getRight(), dashBounds.getY(), dashBounds.getRight(), dashBounds.getBottom()}, dash, 2, 1.0f);
    g.drawDashedLine({dashBounds.getRight(), dashBounds.getBottom(), dashBounds.getX(), dashBounds.getBottom()}, dash, 2, 1.0f);
    g.drawDashedLine({dashBounds.getX(), dashBounds.getBottom(), dashBounds.getX(), dashBounds.getY()}, dash, 2, 1.0f);

    auto content = bounds.reduced(14.0f, 10.0f);
    auto hintBounds = content.removeFromBottom(14.0f);
    content.removeFromBottom(4.0f);

    auto iconBounds = content.removeFromTop(juce::jmin(32.0f, content.getHeight() * 0.42f))
                          .withSizeKeepingCentre(28.0f, 28.0f);
    drawRigIcon(g, iconBounds, icon, isDragOver || isLoaded ? categoryColour : ProfilerStyle::Colors::caption);

    g.setColour(ProfilerStyle::Colors::caption);
    g.setFont(getUiFont(10.0f));
    auto titleBounds = content.removeFromTop(15.0f);
    g.drawFittedText(title, titleBounds.toNearestInt(), juce::Justification::centred, 1);

    const auto detailFont = getUiFont(isLoaded ? 13.0f : 11.0f, isLoaded ? juce::Font::bold : juce::Font::plain);
    g.setFont(detailFont);
    g.setColour(isLoaded ? ProfilerStyle::Colors::text : ProfilerStyle::Colors::caption);
    g.drawFittedText(ellipsize(detailFont, detail, content.getWidth()),
                     content.toNearestInt(),
                     juce::Justification::centred,
                     2);

    g.setFont(getUiFont(9.0f));
    g.setColour(ProfilerStyle::Colors::caption.withAlpha(0.85f));
    g.drawFittedText("Left click / Drag & Drop to load  •  Right click to unload",
                     hintBounds.toNearestInt(),
                     juce::Justification::centred,
                     1);
}

void CustomLookAndFeel::drawGearIcon(juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour colour) const {
    const auto centre = bounds.getCentre();
    const auto outer = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.38f;
    const auto inner = outer * 0.62f;
    const auto hole = outer * 0.28f;
    constexpr int teeth = 8;

    juce::Path gear;
    for (int i = 0; i < teeth; ++i) {
        const auto angle = juce::MathConstants<float>::twoPi * static_cast<float>(i) / static_cast<float>(teeth) - juce::MathConstants<float>::halfPi;
        const auto next = angle + juce::MathConstants<float>::twoPi / static_cast<float>(teeth);
        const auto toothWidth = (next - angle) * 0.28f;

        const juce::Point<float> p0(centre.x + inner * std::cos(angle + toothWidth),
                                    centre.y + inner * std::sin(angle + toothWidth));
        const juce::Point<float> p1(centre.x + outer * std::cos(angle + toothWidth * 1.35f),
                                    centre.y + outer * std::sin(angle + toothWidth * 1.35f));
        const juce::Point<float> p2(centre.x + outer * std::cos(next - toothWidth * 1.35f),
                                    centre.y + outer * std::sin(next - toothWidth * 1.35f));
        const juce::Point<float> p3(centre.x + inner * std::cos(next - toothWidth),
                                    centre.y + inner * std::sin(next - toothWidth));

        if (i == 0) {
            gear.startNewSubPath(p0);
        } else {
            gear.lineTo(p0);
        }
        gear.lineTo(p1);
        gear.lineTo(p2);
        gear.lineTo(p3);
    }
    gear.closeSubPath();
    gear.addEllipse(centre.x - hole, centre.y - hole, hole * 2.0f, hole * 2.0f);
    gear.setUsingNonZeroWinding(false);

    g.setColour(colour);
    g.fillPath(gear);
}

void CustomLookAndFeel::drawLibraryIcon(juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour colour) const {
    auto area = bounds.reduced(bounds.getWidth() * 0.22f, bounds.getHeight() * 0.22f);
    const auto gap = 3.0f;
    const auto cellW = (area.getWidth() - gap) * 0.5f;
    const auto cellH = (area.getHeight() - gap) * 0.5f;

    g.setColour(colour);
    g.fillRoundedRectangle(area.getX(), area.getY(), cellW, cellH, 1.5f);
    g.fillRoundedRectangle(area.getX() + cellW + gap, area.getY(), cellW, cellH, 1.5f);
    g.fillRoundedRectangle(area.getX(), area.getY() + cellH + gap, cellW, cellH, 1.5f);
    g.fillRoundedRectangle(area.getX() + cellW + gap, area.getY() + cellH + gap, cellW, cellH, 1.5f);
}
