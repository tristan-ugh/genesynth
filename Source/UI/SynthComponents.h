#pragma once
#include <JuceHeader.h>
#include "../Style/GeneSynthLookAndFeel.h"
#include "Visualizers.h"

namespace genesynth {

// Helper class to group a Slider and its Attachment
struct LabeledSlider {
    std::unique_ptr<juce::Slider> slider;
    std::unique_ptr<juce::Label> label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

class BaseSection : public juce::Component, public juce::Slider::Listener {
public:
    BaseSection(const juce::String& name, juce::AudioProcessorValueTreeState& apvts);
    void paint(juce::Graphics& g) override;
    void sliderValueChanged(juce::Slider* slider) override;
    
protected:
    void addKnob(LabeledSlider& ls, const juce::String& paramId, const juce::String& labelText, bool isBipolar = false);
    void drawModulableBlock(juce::Graphics& g, LabeledSlider& base, LabeledSlider& mod);
    juce::String sectionName;
    juce::AudioProcessorValueTreeState& apvts;
    juce::Rectangle<int> visBounds;
    float customBgHeight = -1.0f;
public:
    float getCustomBgHeight() const { return customBgHeight; }
};

class OscASection : public BaseSection {
public:
    OscASection(juce::AudioProcessorValueTreeState& apvts);
    void paint(juce::Graphics& g) override;
    void resized() override;
private:
    LabeledSlider wave, ratio;
    LabeledSlider volBase, volMod;
    LabeledSlider freqBase, freqMod;
    LabeledSlider fmBase, fmMod;
    std::unique_ptr<OscVisualizer> oscVis;
};

class OscBSection : public BaseSection {
public:
    OscBSection(juce::AudioProcessorValueTreeState& apvts);
    void paint(juce::Graphics& g) override;
    void resized() override;
private:
    LabeledSlider wave, ratio;
    LabeledSlider volBase, volMod;
    LabeledSlider freqBase, freqMod;
    std::unique_ptr<OscVisualizer> oscVis;
};

class NoiseSection : public BaseSection {
public:
    NoiseSection(juce::AudioProcessorValueTreeState& apvts);
    void paint(juce::Graphics& g) override;
    void resized() override;
private:
    LabeledSlider type;
    LabeledSlider volBase, volMod;
    LabeledSlider freqBase, freqMod;
    std::unique_ptr<NoiseVisualizer> noiseVis;
};

class FilterSectionUI : public BaseSection {
public:
    FilterSectionUI(juce::AudioProcessorValueTreeState& apvts);
    void paint(juce::Graphics& g) override;
    void resized() override;
private:
    LabeledSlider type, res;
    LabeledSlider cutoffBase, cutoffMod;
    LabeledSlider mixBase, mixMod;
    std::unique_ptr<FilterVisualizer> filterVis;
};

class ModulationSectionUI : public BaseSection {
public:
    ModulationSectionUI(juce::AudioProcessorValueTreeState& apvts);
    void resized() override;
private:
    LabeledSlider envA, envD, envS, envR;
    LabeledSlider lfoAmt, lfoFreq;
    std::unique_ptr<AdsrVisualizer> adsrVis;
    std::unique_ptr<LfoVisualizer> lfoVis;
};

class GlobalEnvSection : public BaseSection {
public:
    GlobalEnvSection(juce::AudioProcessorValueTreeState& apvts);
    void resized() override;
private:
    LabeledSlider envA, envD, envS, envR;
    std::unique_ptr<AdsrVisualizer> adsrVis;
};

class SoftClipperSection : public BaseSection {
public:
    SoftClipperSection(juce::AudioProcessorValueTreeState& apvts);
    void resized() override;
private:
    LabeledSlider pre, thr;
};

class ReverbSection : public BaseSection {
public:
    ReverbSection(juce::AudioProcessorValueTreeState& apvts);
    void resized() override;
private:
    LabeledSlider dec, size, mix;
};

} // namespace genesynth
