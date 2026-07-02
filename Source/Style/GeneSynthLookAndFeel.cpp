#include "GeneSynthLookAndFeel.h"

namespace genesynth {

GeneSynthLookAndFeel::GeneSynthLookAndFeel() {
    setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xff181818)); // Very dark grey
    setColour(juce::Slider::trackColourId, juce::Colour(0xff2a2a2a));
    setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff00ffff)); // Default cyan
}

void GeneSynthLookAndFeel::setSliderStyle(juce::Slider& slider, bool isBipolar, juce::Colour accentColor) {
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setColour(juce::Slider::rotarySliderFillColourId, accentColor);
    if (isBipolar) {
        // Just a hint for our custom drawing
        slider.getProperties().set("isBipolar", true);
    }
}

void GeneSynthLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                                            const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) {
    auto radius = (float) juce::jmin(width / 2, height / 2) - 4.0f;
    auto centreX = (float) x + (float) width  * 0.5f;
    auto centreY = (float) y + (float) height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Background track
    g.setColour(slider.findColour(juce::Slider::trackColourId));
    g.fillEllipse(rx, ry, rw, rw);

    // Fill arc
    bool isBipolar = slider.getProperties().getWithDefault("isBipolar", false);
    juce::Path filledArc;
    if (isBipolar) {
        float centerAngle = (rotaryStartAngle + rotaryEndAngle) * 0.5f;
        filledArc.addCentredArc(centreX, centreY, radius, radius, 0.0f, centerAngle, angle, true);
    } else {
        filledArc.addCentredArc(centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, angle, true);
    }
    g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
    g.strokePath(filledArc, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Outline
    g.setColour(juce::Colour(0xff3a3a3a));
    g.drawEllipse(rx, ry, rw, rw, 1.5f);

    // Pointer
    juce::Path p;
    auto pointerLength = radius * 0.33f;
    auto pointerThickness = 2.0f;
    p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
    p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
    g.setColour(juce::Colours::white);
    g.fillPath(p);
}

} // namespace genesynth
