#include "SynthView.h"

namespace genesynth {

SynthView::SynthView(juce::AudioProcessorValueTreeState& apvts, juce::MidiKeyboardState& keyboardState)
    : midiKeyboard(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    oscA = std::make_unique<OscASection>(apvts);
    oscB = std::make_unique<OscBSection>(apvts);
    noise = std::make_unique<NoiseSection>(apvts);
    filter = std::make_unique<FilterSectionUI>(apvts);
    
    modulation = std::make_unique<ModulationSectionUI>(apvts);
    globalEnv = std::make_unique<GlobalEnvSection>(apvts);
    softClipper = std::make_unique<SoftClipperSection>(apvts);
    reverb = std::make_unique<ReverbSection>(apvts);

    addAndMakeVisible(oscA.get());
    addAndMakeVisible(oscB.get());
    addAndMakeVisible(noise.get());
    addAndMakeVisible(filter.get());
    
    addAndMakeVisible(modulation.get());
    addAndMakeVisible(globalEnv.get());
    addAndMakeVisible(softClipper.get());
    addAndMakeVisible(reverb.get());

    addAndMakeVisible(midiKeyboard);
}

void SynthView::paint(juce::Graphics& g)
{
    for (auto* child : getChildren()) {
        if (auto* section = dynamic_cast<BaseSection*>(child)) {
            auto bounds = child->getBounds().toFloat();
            if (section->getCustomBgHeight() > 0) {
                bounds.setHeight(section->getCustomBgHeight());
            }

            juce::Path p;
            p.addRoundedRectangle(bounds, 12.0f);

            // 1. Neumorphic Light Shadow (Top-Left)
            juce::DropShadow lightShadow(juce::Colours::white.withAlpha(0.06f), 5, juce::Point<int>(-2, -2));
            lightShadow.drawForPath(g, p);
            juce::DropShadow diffuseLight(juce::Colours::white.withAlpha(0.03f), 10, juce::Point<int>(-4, -4));
            diffuseLight.drawForPath(g, p);

            // 2. Neumorphic Dark Shadow (Bottom-Right)
            juce::DropShadow darkShadow(juce::Colours::black.withAlpha(0.5f), 5, juce::Point<int>(3, 3));
            darkShadow.drawForPath(g, p);
            juce::DropShadow diffuseDark(juce::Colours::black.withAlpha(0.3f), 10, juce::Point<int>(5, 5));
            diffuseDark.drawForPath(g, p);

            // 3. Flat Base panel color (matches background)
            g.setColour(juce::Colour(0xff0d0d0d));
            g.fillPath(p);
        }
    }
}

void SynthView::paintOverChildren(juce::Graphics& g)
{
    // Draw ambient halos and glowing dots for all rotary sliders at the highest level
    // so they are never clipped by the small section bounds.
    for (auto* sectionComponent : getChildren()) {
        if (auto* section = dynamic_cast<BaseSection*>(sectionComponent)) {
            for (auto* child : section->getChildren()) {
                if (auto* slider = dynamic_cast<juce::Slider*>(child)) {
                    if (slider->getSliderStyle() != juce::Slider::RotaryHorizontalVerticalDrag &&
                        slider->getSliderStyle() != juce::Slider::Rotary) continue;
                    
                    // Get bounds relative to SynthView to draw in our Graphics context
                    auto b = this->getLocalArea(slider, slider->getLocalBounds()).toFloat();
                    float width = b.getWidth();
                    float height = b.getHeight();
                    float radius = std::min(width / 2.0f, height / 2.0f) - 4.0f;
                    float centreX = b.getX() + width * 0.5f;
                    float centreY = b.getY() + height * 0.5f;
                    
                    float rotaryStartAngle = juce::MathConstants<float>::pi * 1.25f;
                    float rotaryEndAngle = juce::MathConstants<float>::pi * 2.75f;
                    
                    float sliderPos = (float)slider->valueToProportionOfLength(slider->getValue());
                    float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
                    
                    float px = centreX + std::sin(angle) * radius;
                    float py = centreY - std::cos(angle) * radius;
                    
                    auto fillColour = slider->findColour(juce::Slider::rotarySliderFillColourId);
                    if (fillColour.isTransparent()) continue;
                    
                    juce::Colour dotBaseColor = fillColour.withSaturation(0.08f).withBrightness(1.0f);
                    
                    // 1. Ambient diffuse halo (propagation zone)
                    float ambientRadius = 18.0f;
                    juce::ColourGradient ambientGrad(dotBaseColor.withAlpha(0.12f), px, py,
                                                     juce::Colours::transparentBlack, px + ambientRadius, py, true);
                    g.setGradientFill(ambientGrad);
                    g.fillEllipse(px - ambientRadius, py - ambientRadius, ambientRadius * 2.0f, ambientRadius * 2.0f);
                    
                    // 2. Luminous Pointer Dot & Intense Glow
                    juce::Path dotPath;
                    dotPath.addEllipse(px - 1.5f, py - 1.5f, 3.0f, 3.0f);
                    
                    juce::DropShadow dotGlow(dotBaseColor.withAlpha(0.9f), 3, juce::Point<int>(0, 0));
                    dotGlow.drawForPath(g, dotPath);
                    
                    g.setColour(dotBaseColor);
                    g.fillPath(dotPath);
                }
            }
        }
    }
}

void SynthView::resized()
{
    auto bounds = getLocalBounds();
    // SynthView now spans the full window to allow shadow bleeding,
    // so we must internally pad the bounds to replicate the original layout:
    bounds.removeFromTop(54); // 16 top margin + 54 = 70 top pad
    bounds.reduce(8, 0);     // 16 side margin + 8 = 24 side pad
    bounds.removeFromBottom(8); // 16 bottom margin + 8 = 24 bottom pad

    // Grid layout
    juce::Grid grid;
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;
    using Px = juce::Grid::Px;

    grid.templateRows = { Track(Px(358)), Track(Fr(1)), Track(Fr(1)) };
    grid.templateColumns = { Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)) };
    grid.columnGap = Px(0);
    grid.rowGap = Px(0);

    auto m1 = juce::GridItem::Margin(8, 8, 0, 8);
    auto m2 = juce::GridItem::Margin(0, 8, 8, 8);
    auto m3 = juce::GridItem::Margin(8, 8, 8, 8);

    auto gridBounds = bounds.removeFromTop(bounds.getHeight() - 70);
    midiKeyboard.setBounds(bounds.withTrimmedTop(10));

    // Generators Row (Row 1)
    grid.items.add(juce::GridItem(oscA.get()).withArea(1, 1, 2, 2).withMargin(m1));
    grid.items.add(juce::GridItem(oscB.get()).withArea(1, 2, 2, 3).withMargin(m1));
    grid.items.add(juce::GridItem(noise.get()).withArea(1, 3, 2, 4).withMargin(m1));
    grid.items.add(juce::GridItem(filter.get()).withArea(1, 4, 2, 5).withMargin(m1));
    // The 5th column is empty in row 1 for now

    // Modulation Row (Row 2)
    // Spans from col 1 to col 4
    grid.items.add(juce::GridItem(modulation.get()).withArea(2, 1, 3, 5).withMargin(m2));
    // Soft Clipper on the right
    grid.items.add(juce::GridItem(softClipper.get()).withArea(2, 5, 3, 6).withMargin(m2));

    // Global & Effects Row (Row 3)
    // Global Envelope spans from col 1 to 4
    grid.items.add(juce::GridItem(globalEnv.get()).withArea(3, 1, 4, 5).withMargin(m3));
    // Reverb on the right
    grid.items.add(juce::GridItem(reverb.get()).withArea(3, 5, 4, 6).withMargin(m3));

    grid.performLayout(gridBounds);
}

} // namespace genesynth
