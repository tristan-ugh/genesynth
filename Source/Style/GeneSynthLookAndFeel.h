#pragma once
#include <JuceHeader.h>

namespace genesynth {

class GeneSynthLookAndFeel : public juce::LookAndFeel_V4 {
public:
    GeneSynthLookAndFeel();
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                          const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override;
    
    // Custom method to set specific colors for bipolar or standard knobs
    static void setSliderStyle(juce::Slider& slider, bool isBipolar, juce::Colour accentColor);
};

} // namespace genesynth
