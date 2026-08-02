#include "GeneSynthLookAndFeel.h"

namespace genesynth {

GeneSynthLookAndFeel::GeneSynthLookAndFeel() {
    setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xff0d0d0d)); // Very dark grey
    setColour(juce::Slider::trackColourId, juce::Colour(0xff1a1a1a));
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

    // Drop Shadow under the knob
    juce::DropShadow shadow(juce::Colours::black.withAlpha(0.6f), 3, juce::Point<int>(0, 2));
    juce::Path shadowPath;
    shadowPath.addEllipse(rx, ry, rw, rw);
    shadow.drawForPath(g, shadowPath);

    // Background track (3D Gradient)
    juce::ColourGradient bgGradient(juce::Colour(0xff222222), centreX, ry, 
                                    juce::Colour(0xff111111), centreX, ry + rw, false);
    g.setGradientFill(bgGradient);
    g.fillEllipse(rx, ry, rw, rw);

    // Inner shadow for 3D rim effect (darker instead of lighter)
    g.setColour(juce::Colours::black.withAlpha(0.6f));
    g.drawEllipse(rx, ry, rw, rw, 1.0f);

    // Fill arc
    bool isBipolar = slider.getProperties().getWithDefault("isBipolar", false);
    juce::Path filledArc;
    if (isBipolar) {
        float centerAngle = (rotaryStartAngle + rotaryEndAngle) * 0.5f;
        filledArc.addCentredArc(centreX, centreY, radius, radius, 0.0f, centerAngle, angle, true);
    } else {
        filledArc.addCentredArc(centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, angle, true);
    }

    auto fillColour = slider.findColour(juce::Slider::rotarySliderFillColourId);
    
    // Glow effect (Real Gaussian blur) is now drawn by the parent section to avoid clipping

    // Thin main line (restored to original thicker weight)
    g.setColour(fillColour);
    g.strokePath(filledArc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    
}

} // namespace genesynth
