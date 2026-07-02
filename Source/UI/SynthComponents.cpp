#include "SynthComponents.h"

namespace genesynth {

BaseSection::BaseSection(const juce::String& name, juce::AudioProcessorValueTreeState& state) 
    : sectionName(name), apvts(state) {}

void BaseSection::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    // Background
    g.setColour(juce::Colour(0xff222222));
    g.fillRoundedRectangle(bounds, 12.0f);
    // Border
    g.setColour(juce::Colour(0xff444444));
    g.drawRoundedRectangle(bounds, 12.0f, 2.0f);
    // Title
    g.setColour(juce::Colours::white);
    g.setFont(16.0f);
    g.drawText(sectionName, bounds.removeFromTop(25.0f), juce::Justification::centred, false);
}

void BaseSection::addKnob(LabeledSlider& ls, const juce::String& paramId, const juce::String& labelText, bool isBipolar) {
    ls.slider = std::make_unique<juce::Slider>();
    GeneSynthLookAndFeel::setSliderStyle(*ls.slider, isBipolar, 
        isBipolar ? juce::Colours::orange : juce::Colours::cyan);
    addAndMakeVisible(*ls.slider);

    ls.label = std::make_unique<juce::Label>("", labelText);
    ls.label->setJustificationType(juce::Justification::centred);
    ls.label->setFont(12.0f);
    addAndMakeVisible(*ls.label);

    ls.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, paramId, *ls.slider);
}

// -----------------------------------------------------------------------------
OscASection::OscASection(juce::AudioProcessorValueTreeState& state) : BaseSection("osc. a", state) {
    addKnob(wave, "osc_a_wave", "wave");
    addKnob(ratio, "osc_a_ratio", "ratio");
    addKnob(volBase, "osc_a_vol_base", "vol");
    addKnob(volMod, "osc_a_vol_mod", "-/+", true);
    addKnob(freqBase, "osc_a_freq_base", "freq");
    addKnob(freqMod, "osc_a_freq_mod", "-/+", true);
    addKnob(fmBase, "osc_a_fm_base", "FM");
    addKnob(fmMod, "osc_a_fm_mod", "-/+", true);
}

void OscASection::resized() {
    auto b = getLocalBounds().reduced(10);
    b.removeFromTop(15); // title
    
    auto topRow = b.removeFromTop(40);
    wave.slider->setBounds(topRow.removeFromLeft(40));
    wave.label->setBounds(wave.slider->getBounds().withY(wave.slider->getY() - 15).withHeight(15));
    
    topRow.removeFromLeft(10);
    ratio.slider->setBounds(topRow.removeFromLeft(40));
    ratio.label->setBounds(ratio.slider->getBounds().withY(ratio.slider->getY() - 15).withHeight(15));
    
    b.removeFromTop(40); // spacer for graphic
    
    auto midRow = b.removeFromTop(40);
    auto botRow = b.removeFromTop(40);
    
    int w = 35;
    int gap = 5;
    
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
    addKnob(wave, "osc_b_wave", "wave");
    addKnob(ratio, "osc_b_ratio", "ratio");
    addKnob(volBase, "osc_b_vol_base", "vol");
    addKnob(volMod, "osc_b_vol_mod", "-/+", true);
    addKnob(freqBase, "osc_b_freq_base", "freq");
    addKnob(freqMod, "osc_b_freq_mod", "-/+", true);
}

void OscBSection::resized() {
    auto b = getLocalBounds().reduced(10);
    b.removeFromTop(15);
    auto topRow = b.removeFromTop(40);
    wave.slider->setBounds(topRow.removeFromLeft(40));
    wave.label->setBounds(wave.slider->getBounds().withY(wave.slider->getY() - 15).withHeight(15));
    topRow.removeFromLeft(10);
    ratio.slider->setBounds(topRow.removeFromLeft(40));
    ratio.label->setBounds(ratio.slider->getBounds().withY(ratio.slider->getY() - 15).withHeight(15));
    
    b.removeFromTop(40);
    auto midRow = b.removeFromTop(40);
    auto botRow = b.removeFromTop(40);
    
    int w = 35, gap = 5;
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
    addKnob(type, "noise_type", "type");
    addKnob(volBase, "noise_vol_base", "vol");
    addKnob(volMod, "noise_vol_mod", "-/+", true);
    addKnob(freqBase, "noise_freq_base", "freq");
    addKnob(freqMod, "noise_freq_mod", "-/+", true);
}

void NoiseSection::resized() {
    auto b = getLocalBounds().reduced(10);
    b.removeFromTop(15);
    auto topRow = b.removeFromTop(40);
    type.slider->setBounds(topRow.removeFromLeft(40));
    type.label->setBounds(type.slider->getBounds().withY(type.slider->getY() - 15).withHeight(15));
    
    b.removeFromTop(40);
    auto midRow = b.removeFromTop(40);
    auto botRow = b.removeFromTop(40);
    
    int w = 35, gap = 5;
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
    addKnob(type, "filter_type", "type");
    addKnob(res, "filter_res", "res.");
    addKnob(cutoffBase, "filter_cutoff_base", "freq");
    addKnob(cutoffMod, "filter_cutoff_mod", "-/+", true);
    addKnob(mixBase, "filter_mix_base", "mix");
    addKnob(mixMod, "filter_mix_mod", "-/+", true);
}

void FilterSectionUI::resized() {
    auto b = getLocalBounds().reduced(10);
    b.removeFromTop(15);
    auto topRow = b.removeFromTop(40);
    type.slider->setBounds(topRow.removeFromLeft(40));
    type.label->setBounds(type.slider->getBounds().withY(type.slider->getY() - 15).withHeight(15));
    topRow.removeFromLeft(10);
    res.slider->setBounds(topRow.removeFromLeft(40));
    res.label->setBounds(res.slider->getBounds().withY(res.slider->getY() - 15).withHeight(15));
    
    b.removeFromTop(40);
    auto midRow = b.removeFromTop(40);
    auto botRow = b.removeFromTop(40);
    
    int w = 35, gap = 5;
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
    addKnob(lfoAmt, "lfo_amt", "amt");
    addKnob(lfoFreq, "lfo_freq", "Hz");
}

void ModulationSectionUI::resized() {
    auto b = getLocalBounds().reduced(10);
    b.removeFromTop(20);
    
    auto leftBlock = b.removeFromLeft(200);
    int w = 35, gap = 5;
    
    envA.slider->setBounds(leftBlock.removeFromLeft(w).withHeight(40));
    leftBlock.removeFromLeft(gap);
    envD.slider->setBounds(leftBlock.removeFromLeft(w).withHeight(40));
    leftBlock.removeFromLeft(gap);
    envS.slider->setBounds(leftBlock.removeFromLeft(w).withHeight(40));
    leftBlock.removeFromLeft(gap);
    envR.slider->setBounds(leftBlock.removeFromLeft(w).withHeight(40));
    
    auto rightBlock = b.removeFromRight(100);
    lfoAmt.slider->setBounds(rightBlock.removeFromLeft(w).withHeight(40));
    rightBlock.removeFromLeft(gap);
    lfoFreq.slider->setBounds(rightBlock.removeFromLeft(w).withHeight(40));
}

// -----------------------------------------------------------------------------
GlobalEnvSection::GlobalEnvSection(juce::AudioProcessorValueTreeState& state) : BaseSection("global enveloppe", state) {
    addKnob(envA, "env_global_a", "A");
    addKnob(envD, "env_global_d", "D");
    addKnob(envS, "env_global_s", "S");
    addKnob(envR, "env_global_r", "R");
}
void GlobalEnvSection::resized() {
    auto b = getLocalBounds().reduced(10);
    b.removeFromTop(20);
    auto leftBlock = b.removeFromLeft(200);
    int w = 35, gap = 5;
    envA.slider->setBounds(leftBlock.removeFromLeft(w).withHeight(40)); leftBlock.removeFromLeft(gap);
    envD.slider->setBounds(leftBlock.removeFromLeft(w).withHeight(40)); leftBlock.removeFromLeft(gap);
    envS.slider->setBounds(leftBlock.removeFromLeft(w).withHeight(40)); leftBlock.removeFromLeft(gap);
    envR.slider->setBounds(leftBlock.removeFromLeft(w).withHeight(40));
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
