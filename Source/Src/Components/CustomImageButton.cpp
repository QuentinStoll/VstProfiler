#include "Components/CustomImageButton.h"

//=============================================================================
// CustomImageButton Implementation
//=============================================================================

CustomImageButton::CustomImageButton(const juce::String& buttonName,
                                     juce::Image image,
                                     ProfilerStyle::Theme theme) : juce::Button(buttonName),
                                                                   _image(image) {
    setTheme(theme);
}

CustomImageButton::CustomImageButton(const juce::String& buttonName,
                                     std::unique_ptr<juce::Drawable> drawable,
                                     ProfilerStyle::Theme theme) : juce::Button(buttonName),
                                                                   _drawable(std::move(drawable)) {
    setTheme(theme);
}

CustomImageButton::CustomImageButton(const juce::String& buttonName,
                                     const juce::File& svgFile,
                                     ProfilerStyle::Theme theme) : juce::Button(buttonName),
                                                                   _drawable(juce::Drawable::createFromSVGFile(svgFile)) {
    if (_drawable != nullptr) {
        _drawable->replaceColour(juce::Colour(0xff000000), ProfilerStyle::Colors::white);
    }
    setTheme(theme);
}

void CustomImageButton::setTheme(ProfilerStyle::Theme theme) {
    switch (theme) {
        case ProfilerStyle::Theme::Darker:
            _backgroundColour = ProfilerStyle::Colors::darkestGrey;
            setOutlineVisible(true);
            break;
        case ProfilerStyle::Theme::Dark:
            _backgroundColour = ProfilerStyle::Colors::darkerGrey;
            break;
        case ProfilerStyle::Theme::Light:
            _backgroundColour = ProfilerStyle::Colors::darkGrey;
            break;
        case ProfilerStyle::Theme::Orange:
            _backgroundColour = ProfilerStyle::Colors::orange;
            break;
    }
}

void CustomImageButton::paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) {
    auto area = getLocalBounds();
    auto cornerSize = 4.0f;
    auto fillArea = _outlineVisible ? area.reduced(2) : area;

    auto buttonText = getButtonText();
    auto hasImage = hasImageContent();
    auto hasText = buttonText.isNotEmpty();
    auto gap = 0;
    auto font = juce::Font(juce::FontOptions(16.0f));
    auto contentArea = area.reduced(10);
    auto imageArea = hasText ? contentArea : contentArea.reduced(juce::roundToInt(getWidth() * 0.3f));
    auto textArea = contentArea;

    if (hasImage && hasText) {
        gap = 10;
        auto removedHeight = juce::roundToInt(contentArea.getHeight() * 0.25f);
        contentArea.removeFromTop(removedHeight);
        contentArea.removeFromBottom(removedHeight);
        auto textHeight = juce::roundToInt(font.getHeight()) + 2;
        textArea = contentArea.removeFromBottom(juce::jmin(textHeight, contentArea.getHeight()));
        imageArea = contentArea;
    }

    auto baseColour = _backgroundColour;
    if (isButtonDown)
        baseColour = baseColour.darker(0.2f);
    else if (isMouseOverButton)
        baseColour = baseColour.brighter(0.1f);

    g.setGradientFill(ProfilerStyle::Gradients::vertical(
        area.toFloat(),
        baseColour.brighter(0.2f),
        baseColour.darker(0.2f),
        0.9f));
    g.fillRoundedRectangle(fillArea.toFloat(), cornerSize);

    if (_outlineVisible) {
        g.setColour(_outlineColour);
        g.drawRoundedRectangle(area.toFloat(), cornerSize + 2.0f, 3.0f);
    }

    auto alpha = (isEnabled() ? 1.0f : 0.5f) * (isButtonDown ? 0.8f : 1.0f);

    if (hasImage) {
        drawImageContent(g, imageArea.reduced(0, gap).toFloat(), alpha);
    }

    if (hasText) {
        g.setFont(font);
        g.setColour(_textColour.withMultipliedAlpha(alpha));
        g.drawText(buttonText, textArea, juce::Justification::centred, true);
    }
}

void CustomImageButton::drawImageContent(juce::Graphics& g, juce::Rectangle<float> imageArea, float alpha) {
    if (_drawable != nullptr) {
        _drawable->drawWithin(g,
                              imageArea,
                              juce::RectanglePlacement::centred,
                              alpha);
        return;
    }

    if (_image.isValid()) {
        g.setOpacity(alpha);
        g.drawImageWithin(_image,
                          static_cast<int>(imageArea.getX()),
                          static_cast<int>(imageArea.getY()),
                          static_cast<int>(imageArea.getWidth()),
                          static_cast<int>(imageArea.getHeight()),
                          juce::RectanglePlacement::centred);
        g.setOpacity(1.0f);
    }
}
