#include "SynthComponents.h"

namespace genesynth {

namespace {
    void drawInnerShadow(juce::Graphics& g, const juce::Path& path, juce::Colour shadowColour, int shadowRadius, juce::Point<int> offset)
    {
        juce::Path shadowPath;
        auto bounds = path.getBounds().expanded(shadowRadius + std::abs(offset.x) + 10, shadowRadius + std::abs(offset.y) + 10);
        shadowPath.addRectangle(bounds);
        shadowPath.setUsingNonZeroWinding(false);
        shadowPath.addPath(path);

        juce::DropShadow shadow(shadowColour, shadowRadius, offset);
        g.saveState();
        g.reduceClipRegion(path);
        shadow.drawForPath(g, shadowPath);
        g.restoreState();
    }
}



BaseSection::BaseSection(const juce::String& name, juce::AudioProcessorValueTreeState& state) 
    : sectionName(name), apvts(state) {}

void BaseSection::sliderValueChanged(juce::Slider* slider) {
    if (slider != nullptr) {
        // Force the parent (SynthView) to repaint the expanded area around the slider
        if (auto* parent = getParentComponent()) {
            parent->repaint(parent->getLocalArea(slider, slider->getLocalBounds()).expanded(30));
        }
    }
}

void BaseSection::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    auto bgBounds = bounds;
    if (customBgHeight > 0) {
        bgBounds.setHeight(customBgHeight);
    }
    
    // Title
    g.setColour(juce::Colours::white);
    g.setFont(16.0f);
    g.drawText(sectionName, bgBounds.removeFromTop(25.0f), juce::Justification::centred, false);
    
    if (!visBounds.isEmpty()) {
        // Dummy inner shadow removed because visualizers now handle their own backgrounds.
        // Child visualizers are responsible for drawing within their bounds.
    }
}



void BaseSection::drawModulableBlock(juce::Graphics& g, LabeledSlider& base, LabeledSlider& mod) {
    if (base.slider == nullptr || mod.slider == nullptr || base.label == nullptr) return;

    auto b1 = base.slider->getBounds();
    auto b2 = mod.slider->getBounds();
    auto labelBounds = base.label->getBounds();

    // Track connection
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    int cx = b1.getCentreX();

    // Top segment (from bottom of the main panel at 245 to the top of the label)
    g.drawLine((float)cx, 245.0f, (float)cx, (float)labelBounds.getY(), 2.0f);
    
    // Bottom segment (from bottom of label, through sliders, to the bottom of the component bounds to touch modulation)
    g.drawLine((float)cx, (float)labelBounds.getBottom(), (float)cx, (float)b2.getBottom() + 5.0f, 2.0f);
}

void BaseSection::addKnob(LabeledSlider& ls, const juce::String& paramId, const juce::String& labelText, bool isBipolar) {
    ls.slider = std::make_unique<juce::Slider>();
    GeneSynthLookAndFeel::setSliderStyle(*ls.slider, isBipolar, 
        isBipolar ? juce::Colours::orange : juce::Colours::cyan);
    addAndMakeVisible(*ls.slider);
    ls.slider->addListener(this);

    ls.label = std::make_unique<juce::Label>("", labelText);
    ls.label->setJustificationType(juce::Justification::centred);
    ls.label->setFont(12.0f);
    addAndMakeVisible(*ls.label);

    ls.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, paramId, *ls.slider);
}

// -----------------------------------------------------------------------------
OscASection::OscASection(juce::AudioProcessorValueTreeState& state) : BaseSection("osc. a", state) {
    customBgHeight = 245.0f;
    addKnob(wave, "osc_a_wave", "wave");
    addKnob(ratio, "osc_a_ratio", "ratio");
    addKnob(volBase, "osc_a_vol_base", "vol");
    addKnob(volMod, "osc_a_vol_mod", "-/+", true);
    addKnob(freqBase, "osc_a_freq_base", "freq");
    addKnob(freqMod, "osc_a_freq_mod", "-/+", true);
    addKnob(fmBase, "osc_a_fm_base", "FM");
    addKnob(fmMod, "osc_a_fm_mod", "-/+", true);
    
    oscVis = std::make_unique<OscVisualizer>(apvts, "osc_a_wave", "osc_a_ratio", juce::Colours::cyan, "osc_a_fm_base", "osc_b_wave", "osc_b_ratio");
    addAndMakeVisible(*oscVis);
}

void OscASection::paint(juce::Graphics& g) {
    BaseSection::paint(g);
    drawModulableBlock(g, volBase, volMod);
    drawModulableBlock(g, freqBase, freqMod);
    drawModulableBlock(g, fmBase, fmMod);
}

void OscASection::resized() {
    auto b = getLocalBounds().reduced(10);
    b.removeFromTop(25); // title
    
    auto topRow = b.removeFromTop(40).withSizeKeepingCentre(90, 40);
    wave.slider->setBounds(topRow.removeFromLeft(40));
    wave.label->setBounds(wave.slider->getBounds().withY(wave.slider->getY() - 15).withHeight(15));
    
    topRow.removeFromLeft(10);
    ratio.slider->setBounds(topRow.removeFromLeft(40));
    ratio.label->setBounds(ratio.slider->getBounds().withY(ratio.slider->getY() - 15).withHeight(15));
    
    int midY = 265;
    int w = 35;
    int gap = 18;
    int totalW = (w * 3) + (gap * 2);
    
    int sq = getWidth() - 40;
    visBounds = juce::Rectangle<int>(20, 85, sq, sq);
    oscVis->setBounds(visBounds);
    
    auto midRow = juce::Rectangle<int>(0, midY, getWidth(), 40).withSizeKeepingCentre(totalW, 40);
    auto botRow = juce::Rectangle<int>(0, midY + 40, getWidth(), 40).withSizeKeepingCentre(totalW, 40);
    
    volBase.slider->setBounds(midRow.removeFromLeft(w));
    volBase.label->setBounds(volBase.slider->getBounds().withY(volBase.slider->getY() - 15).withHeight(15));
    volMod.slider->setBounds(botRow.removeFromLeft(w));
    
    midRow.removeFromLeft(gap); botRow.removeFromLeft(gap);
    freqBase.slider->setBounds(midRow.removeFromLeft(w));
    freqBase.label->setBounds(freqBase.slider->getBounds().withY(freqBase.slider->getY() - 15).withHeight(15));
    freqMod.slider->setBounds(botRow.removeFromLeft(w));
    
    midRow.removeFromLeft(gap); botRow.removeFromLeft(gap);
    fmBase.slider->setBounds(midRow.removeFromLeft(w));
    fmBase.label->setBounds(fmBase.slider->getBounds().withY(fmBase.slider->getY() - 15).withHeight(15));
    fmMod.slider->setBounds(botRow.removeFromLeft(w));
}

// -----------------------------------------------------------------------------
OscBSection::OscBSection(juce::AudioProcessorValueTreeState& state) : BaseSection("osc. b", state) {
    customBgHeight = 245.0f;
    addKnob(wave, "osc_b_wave", "wave");
    addKnob(ratio, "osc_b_ratio", "ratio");
    addKnob(volBase, "osc_b_vol_base", "vol");
    addKnob(volMod, "osc_b_vol_mod", "-/+", true);
    addKnob(freqBase, "osc_b_freq_base", "freq");
    addKnob(freqMod, "osc_b_freq_mod", "-/+", true);
    
    oscVis = std::make_unique<OscVisualizer>(apvts, "osc_b_wave", "osc_b_ratio", juce::Colours::magenta);
    addAndMakeVisible(*oscVis);
}

void OscBSection::paint(juce::Graphics& g) {
    BaseSection::paint(g);
    drawModulableBlock(g, volBase, volMod);
    drawModulableBlock(g, freqBase, freqMod);
}

void OscBSection::resized() {
    auto b = getLocalBounds().reduced(10);
    b.removeFromTop(25);
    auto topRow = b.removeFromTop(40).withSizeKeepingCentre(90, 40);
    wave.slider->setBounds(topRow.removeFromLeft(40));
    wave.label->setBounds(wave.slider->getBounds().withY(wave.slider->getY() - 15).withHeight(15));
    
    topRow.removeFromLeft(10);
    ratio.slider->setBounds(topRow.removeFromLeft(40));
    ratio.label->setBounds(ratio.slider->getBounds().withY(ratio.slider->getY() - 15).withHeight(15));
    int midY = 265;
    int w = 35;
    int gap = 18;
    int totalW = (w * 2) + gap;
    
    int sq = getWidth() - 40;
    visBounds = juce::Rectangle<int>(20, 85, sq, sq);
    oscVis->setBounds(visBounds);
    
    auto midRow = juce::Rectangle<int>(0, midY, getWidth(), 40).withSizeKeepingCentre(totalW, 40);
    auto botRow = juce::Rectangle<int>(0, midY + 40, getWidth(), 40).withSizeKeepingCentre(totalW, 40);
    volBase.slider->setBounds(midRow.removeFromLeft(w));
    volBase.label->setBounds(volBase.slider->getBounds().withY(volBase.slider->getY() - 15).withHeight(15));
    volMod.slider->setBounds(botRow.removeFromLeft(w));
    
    midRow.removeFromLeft(gap); botRow.removeFromLeft(gap);
    freqBase.slider->setBounds(midRow.removeFromLeft(w));
    freqBase.label->setBounds(freqBase.slider->getBounds().withY(freqBase.slider->getY() - 15).withHeight(15));
    freqMod.slider->setBounds(botRow.removeFromLeft(w));
}

// -----------------------------------------------------------------------------
NoiseSection::NoiseSection(juce::AudioProcessorValueTreeState& state) : BaseSection("noise", state) {
    customBgHeight = 245.0f;
    addKnob(type, "noise_type", "type");
    addKnob(volBase, "noise_vol_base", "vol");
    addKnob(volMod, "noise_vol_mod", "-/+", true);
    addKnob(freqBase, "noise_freq_base", "freq");
    addKnob(freqMod, "noise_freq_mod", "-/+", true);
    
    noiseVis = std::make_unique<NoiseVisualizer>(apvts, "noise_type");
    addAndMakeVisible(*noiseVis);
}

void NoiseSection::paint(juce::Graphics& g) {
    BaseSection::paint(g);
    drawModulableBlock(g, volBase, volMod);
    drawModulableBlock(g, freqBase, freqMod);
}

void NoiseSection::resized() {
    auto b = getLocalBounds().reduced(10);
    b.removeFromTop(25);
    auto topRow = b.removeFromTop(40).withSizeKeepingCentre(40, 40);
    type.slider->setBounds(topRow.removeFromLeft(40));
    type.label->setBounds(type.slider->getBounds().withY(type.slider->getY() - 15).withHeight(15));
    int midY = 265;
    int w = 35;
    int gap = 18;
    int totalW = (w * 2) + gap;
    
    int sq = getWidth() - 40;
    visBounds = juce::Rectangle<int>(20, 85, sq, sq);
    noiseVis->setBounds(visBounds);
    
    auto midRow = juce::Rectangle<int>(0, midY, getWidth(), 40).withSizeKeepingCentre(totalW, 40);
    auto botRow = juce::Rectangle<int>(0, midY + 40, getWidth(), 40).withSizeKeepingCentre(totalW, 40);
    volBase.slider->setBounds(midRow.removeFromLeft(w));
    volBase.label->setBounds(volBase.slider->getBounds().withY(volBase.slider->getY() - 15).withHeight(15));
    volMod.slider->setBounds(botRow.removeFromLeft(w));
    
    midRow.removeFromLeft(gap); botRow.removeFromLeft(gap);
    freqBase.slider->setBounds(midRow.removeFromLeft(w));
    freqBase.label->setBounds(freqBase.slider->getBounds().withY(freqBase.slider->getY() - 15).withHeight(15));
    freqMod.slider->setBounds(botRow.removeFromLeft(w));
}

// -----------------------------------------------------------------------------
FilterSectionUI::FilterSectionUI(juce::AudioProcessorValueTreeState& state) : BaseSection("filter", state) {
    customBgHeight = 245.0f;
    addKnob(type, "filter_type", "type");
    addKnob(res, "filter_res", "res.");
    addKnob(cutoffBase, "filter_cutoff_base", "freq");
    addKnob(cutoffMod, "filter_cutoff_mod", "-/+", true);
    addKnob(mixBase, "filter_mix_base", "mix");
    addKnob(mixMod, "filter_mix_mod", "-/+", true);
    
    filterVis = std::make_unique<FilterVisualizer>(apvts, "filter_type", "filter_res", "filter_cutoff_base");
    addAndMakeVisible(*filterVis);
}

void FilterSectionUI::paint(juce::Graphics& g) {
    BaseSection::paint(g);
    drawModulableBlock(g, cutoffBase, cutoffMod);
    drawModulableBlock(g, mixBase, mixMod);
}

void FilterSectionUI::resized() {
    auto b = getLocalBounds().reduced(10);
    b.removeFromTop(25);
    auto topRow = b.removeFromTop(40).withSizeKeepingCentre(90, 40);
    type.slider->setBounds(topRow.removeFromLeft(40));
    type.label->setBounds(type.slider->getBounds().withY(type.slider->getY() - 15).withHeight(15));
    topRow.removeFromLeft(10);
    res.slider->setBounds(topRow.removeFromLeft(40));
    res.label->setBounds(res.slider->getBounds().withY(res.slider->getY() - 15).withHeight(15));
    
    int midY = 265;
    int w = 35;
    int gap = 18;
    int totalW = (w * 2) + gap;
    
    int sq = getWidth() - 40;
    visBounds = juce::Rectangle<int>(20, 85, sq, sq);
    filterVis->setBounds(visBounds);
    
    auto midRow = juce::Rectangle<int>(0, midY, getWidth(), 40).withSizeKeepingCentre(totalW, 40);
    auto botRow = juce::Rectangle<int>(0, midY + 40, getWidth(), 40).withSizeKeepingCentre(totalW, 40);
    mixBase.slider->setBounds(midRow.removeFromLeft(w));
    mixBase.label->setBounds(mixBase.slider->getBounds().withY(mixBase.slider->getY() - 15).withHeight(15));
    mixMod.slider->setBounds(botRow.removeFromLeft(w));
    
    midRow.removeFromLeft(gap); botRow.removeFromLeft(gap);
    cutoffBase.slider->setBounds(midRow.removeFromLeft(w));
    cutoffBase.label->setBounds(cutoffBase.slider->getBounds().withY(cutoffBase.slider->getY() - 15).withHeight(15));
    cutoffMod.slider->setBounds(botRow.removeFromLeft(w));
}

// -----------------------------------------------------------------------------
ModulationSectionUI::ModulationSectionUI(juce::AudioProcessorValueTreeState& state) : BaseSection("modulation", state) {
    addKnob(envA, "env_mod_a", "A");
    addKnob(envD, "env_mod_d", "D");
    addKnob(envS, "env_mod_s", "S");
    addKnob(envR, "env_mod_r", "R");
    
    envA.slider->setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff00ff80));
    envD.slider->setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff00ff80));
    envS.slider->setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff00ff80));
    envR.slider->setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff00ff80));

    addKnob(lfoAmt, "lfo_amt", "amt");
    addKnob(lfoFreq, "lfo_freq", "Hz");
    
    lfoAmt.slider->setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::magenta);
    lfoFreq.slider->setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::magenta);

    adsrVis = std::make_unique<AdsrVisualizer>(state, "env_mod_a", "env_mod_d", "env_mod_s", "env_mod_r", juce::Colour(0xff00ff80));
    addAndMakeVisible(*adsrVis);

    lfoVis = std::make_unique<LfoVisualizer>(state, "lfo_freq", "lfo_amt", juce::Colours::magenta);
    addAndMakeVisible(*lfoVis);
}

void ModulationSectionUI::resized() {
    auto b = getLocalBounds().reduced(10);
    b.removeFromTop(20);
    
    auto leftBlock = b.removeFromLeft(180);
    int w = 35, gap = 5;
    
    auto setKnob = [&](LabeledSlider& ls, juce::Rectangle<int>& area) {
        auto k = area.removeFromLeft(w).withSizeKeepingCentre(w, 60);
        ls.slider->setBounds(k.withTrimmedBottom(15));
        ls.label->setBounds(k.withTop(k.getBottom() - 15));
    };

    setKnob(envA, leftBlock); leftBlock.removeFromLeft(gap);
    setKnob(envD, leftBlock); leftBlock.removeFromLeft(gap);
    setKnob(envS, leftBlock); leftBlock.removeFromLeft(gap);
    setKnob(envR, leftBlock);
    
    auto rightBlock = b.removeFromRight(90);
    setKnob(lfoAmt, rightBlock); rightBlock.removeFromLeft(gap);
    setKnob(lfoFreq, rightBlock);

    // Remaining space for visualizers
    b.removeFromLeft(10);
    b.removeFromRight(10);
    
    auto lfoArea = b.removeFromRight(150);
    lfoVis->setBounds(lfoArea);
    b.removeFromRight(10);
    adsrVis->setBounds(b);
}

// -----------------------------------------------------------------------------
GlobalEnvSection::GlobalEnvSection(juce::AudioProcessorValueTreeState& state) : BaseSection("global enveloppe", state) {
    addKnob(envA, "env_global_a", "A");
    addKnob(envD, "env_global_d", "D");
    addKnob(envS, "env_global_s", "S");
    addKnob(envR, "env_global_r", "R");
    
    envA.slider->setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff00ff80));
    envD.slider->setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff00ff80));
    envS.slider->setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff00ff80));
    envR.slider->setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff00ff80));

    adsrVis = std::make_unique<AdsrVisualizer>(state, "env_global_a", "env_global_d", "env_global_s", "env_global_r", juce::Colour(0xff00ff80));
    addAndMakeVisible(*adsrVis);
}
void GlobalEnvSection::resized() {
    auto b = getLocalBounds().reduced(10);
    b.removeFromTop(20);

    auto leftBlock = b.removeFromLeft(180);
    int w = 35, gap = 5;
    
    auto setKnob = [&](LabeledSlider& ls, juce::Rectangle<int>& area) {
        auto k = area.removeFromLeft(w).withSizeKeepingCentre(w, 60);
        ls.slider->setBounds(k.withTrimmedBottom(15));
        ls.label->setBounds(k.withTop(k.getBottom() - 15));
    };

    setKnob(envA, leftBlock); leftBlock.removeFromLeft(gap);
    setKnob(envD, leftBlock); leftBlock.removeFromLeft(gap);
    setKnob(envS, leftBlock); leftBlock.removeFromLeft(gap);
    setKnob(envR, leftBlock);

    b.removeFromLeft(10);
    adsrVis->setBounds(b);
}

// -----------------------------------------------------------------------------
SoftClipperSection::SoftClipperSection(juce::AudioProcessorValueTreeState& state) : BaseSection("s. clip", state) {
    addKnob(pre, "clipper_vol", "pre");
    addKnob(thr, "clipper_thresh", "thr");
}
void SoftClipperSection::resized() {
    auto b = getLocalBounds().reduced(10);
    b.removeFromTop(20);
    pre.slider->setBounds(b.removeFromLeft(35).withHeight(40));
    b.removeFromLeft(5);
    thr.slider->setBounds(b.removeFromLeft(35).withHeight(40));
}

// -----------------------------------------------------------------------------
ReverbSection::ReverbSection(juce::AudioProcessorValueTreeState& state) : BaseSection("reverb", state) {
    addKnob(dec, "reverb_decay", "dec");
    addKnob(size, "reverb_size", "size");
    addKnob(mix, "reverb_mix", "mix");
}
void ReverbSection::resized() {
    auto b = getLocalBounds().reduced(10);
    b.removeFromTop(20);
    dec.slider->setBounds(b.removeFromLeft(35).withHeight(40)); b.removeFromLeft(5);
    size.slider->setBounds(b.removeFromLeft(35).withHeight(40)); b.removeFromLeft(5);
    mix.slider->setBounds(b.removeFromLeft(35).withHeight(40));
}

} // namespace genesynth
