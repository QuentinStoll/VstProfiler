#include "Components/CustomImageButton.h"

//=============================================================================
// CustomImageButton Implementation
//=============================================================================

CustomImageButton::CustomImageButton(const juce::String& buttonName,
                                     const void* binaryData,
                                     size_t dataSize,
                                     ProfilerStyle::Theme theme) : juce::Button(buttonName),
                                                                   _drawable(juce::Drawable::createFromImageData(binaryData, dataSize)) {
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
            setOutlineVisible(true);
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
    auto cornerSize = 8.0f;
    
    // Define a native JUCE border size (2.0f pixels thick on all sides)
    const juce::BorderSize<float> buttonBorder { 2.0f };
    
    // 1. Calculate the background fill area (reduced if outline is visible to prevent overlap)
    auto fillArea = _outlineVisible ? area.reduced(static_cast<int>(buttonBorder.getTop())) : area;

    auto buttonText = getButtonText();
    auto hasImage = (_drawable != nullptr);
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

    // 2. Button Background Color Logic
    auto baseColour = _backgroundColour;
    if (isButtonDown)
        baseColour = baseColour.darker(0.2f);
    else if (isMouseOverButton)
        baseColour = baseColour.brighter(0.1f);

    g.setColour(baseColour);
    g.fillRoundedRectangle(fillArea.toFloat(), cornerSize);

    // 3. NATIVE BORDER DRAWING (Method 1)
    if (_outlineVisible) {
        g.setColour(_outlineColour);
        
        // Native JUCE method to shrink the area by the border thickness 
        // to ensure the stroke is drawn perfectly inside the button bounds
        auto outlineArea = buttonBorder.subtractedFrom(area.toFloat());
        g.drawRoundedRectangle(outlineArea, cornerSize, buttonBorder.getTop());
    }

    // 4. Content Alpha & Rendering
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
    }
}
