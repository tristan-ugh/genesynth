#pragma once
#include <JuceHeader.h>
#include "../Style/GeneSynthLookAndFeel.h"

namespace genesynth {

// Helper class to group a Slider and its Attachment
struct LabeledSlider {
    std::unique_ptr<juce::Slider> slider;
    std::unique_ptr<juce::Label> label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

class BaseSection : public juce::Component {
public:
    BaseSection(const juce::String& name, juce::AudioProcessorValueTreeState& apvts);
    void paint(juce::Graphics& g) override;
    
protected:
    void addKnob(LabeledSlider& ls, const juce::String& paramId, const juce::String& labelText, bool isBipolar = false);
    juce::String sectionName;
    juce::AudioProcessorValueTreeState& apvts;
};

class OscASection : public BaseSection {
public:
    OscASection(juce::AudioProcessorValueTreeState& apvts);
    void resized() override;
private:
    LabeledSlider wave, ratio;
    LabeledSlider volBase, volMod;
    LabeledSlider freqBase, freqMod;
    LabeledSlider fmBase, fmMod;
};

class OscBSection : public BaseSection {
public:
    OscBSection(juce::AudioProcessorValueTreeState& apvts);
    void resized() override;
private:
    LabeledSlider wave, ratio;
    LabeledSlider volBase, volMod;
    LabeledSlider freqBase, freqMod;
};

class NoiseSection : public BaseSection {
public:
    NoiseSection(juce::AudioProcessorValueTreeState& apvts);
    void resized() override;
private:
    LabeledSlider type;
    LabeledSlider volBase, volMod;
    LabeledSlider freqBase, freqMod;
};

class FilterSectionUI : public BaseSection {
public:
    FilterSectionUI(juce::AudioProcessorValueTreeState& apvts);
    void resized() override;
private:
    LabeledSlider type, res;
    LabeledSlider cutoffBase, cutoffMod;
    LabeledSlider mixBase, mixMod;
};

class ModulationSectionUI : public BaseSection {
public:
    ModulationSectionUI(juce::AudioProcessorValueTreeState& apvts);
    void resized() override;
private:
    LabeledSlider envA, envD, envS, envR;
    LabeledSlider lfoAmt, lfoFreq;
};

class GlobalEnvSection : public BaseSection {
public:
    GlobalEnvSection(juce::AudioProcessorValueTreeState& apvts);
    void resized() override;
private:
    LabeledSlider envA, envD, envS, envR;
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
