#pragma once

#include <JuceHeader.h>
#include "Styles/Stylesheet.h"

class CustomButtonLF : public juce::LookAndFeel_V4
{
public:
	//
	void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
		bool isMouseOverButton, bool isButtonDown) override
	{
		auto area = button.getLocalBounds().toFloat().reduced(2.0f);
		auto buttonColour = ProfilerStyle::Colors::darkGrey;

		if (isMouseOverButton) buttonColour = buttonColour.brighter(0.1f);
		if (isButtonDown) buttonColour = buttonColour.darker(0.1f);

		g.setGradientFill(ProfilerStyle::Gradients::vertical(
			area,
			buttonColour.brighter(0.1f),
			buttonColour.darker(0.2f),
			0.9f
		));
		g.fillRoundedRectangle(area, 5.0f);
	}

    //
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, const float rotaryStartAngle,
        const float rotaryEndAngle, juce::Slider& slider) override
    {
		// Customize the rotary slider's appearance
        auto outline = slider.findColour(juce::Slider::rotarySliderOutlineColourId);
        auto fill = ProfilerStyle::Colors::orange;
        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(10);
        auto radius = std::min(bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto lineW = 3.0f;
        auto centreX = x + width * 0.5f;
        auto centreY = y + height * 0.5f;
        auto arcRadius = radius - lineW * 2.0f;

		// Draw the background arc (the full range of the slider)
        juce::Path backgroundArc;
        backgroundArc.addCentredArc(centreX, centreY, arcRadius, arcRadius, 0.0f,
            rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(ProfilerStyle::Colors::darkGrey);
        g.strokePath(backgroundArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

		// Only draw the value arc if the slider is enabled
        if (slider.isEnabled())
        {
            juce::Path valueArc;
            valueArc.addCentredArc(centreX, centreY, arcRadius, arcRadius, 0.0f,
                rotaryStartAngle, toAngle, true);
            g.setColour(fill);
            g.strokePath(valueArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

		// Style the knob (a circle in the center of the arc)
        auto knobRadius = arcRadius - 6.0f;
        juce::Rectangle<float> knobBounds(centreX - knobRadius, centreY - knobRadius,
            knobRadius * 2, knobRadius * 2);

		// Create a vertical gradient for the knob
        juce::ColourGradient knobGrad(ProfilerStyle::Colors::lighterGrey, centreX, centreY - knobRadius,
            ProfilerStyle::Colors::darkestGrey, centreX, centreY + knobRadius, false);
        g.setGradientFill(knobGrad);
        g.fillEllipse(knobBounds);

		// Draw the outline of the knob
        g.setColour(ProfilerStyle::Colors::darkerGrey);
        g.drawEllipse(knobBounds, 1.0f);

		// Style the pointer (a small circle in the knob)
        juce::Path p;
        auto pointerRadius = 3.0f;
        auto pointerLength = knobRadius * 0.7f;
        p.addEllipse(-pointerRadius, -pointerLength, pointerRadius * 2, pointerRadius * 2);
        p.applyTransform(juce::AffineTransform::rotation(toAngle).translated(centreX, centreY));
        g.setColour(fill);
        g.fillPath(p);
    }

    //
    void drawLabel(juce::Graphics& g, juce::Label& label) override
    {
        g.setColour(label.findColour(juce::Label::textColourId));
        g.setFont(label.getFont());
        g.drawFittedText(label.getText(), label.getLocalBounds(), juce::Justification::centred, 1);
    }

private:

};