#pragma once

#include <JuceHeader.h>

namespace genesynth {

class AdsrVisualizer : public juce::Component, public juce::Timer
{
public:
    AdsrVisualizer(juce::AudioProcessorValueTreeState& apvts,
                   const juce::String& paramA,
                   const juce::String& paramD,
                   const juce::String& paramS,
                   const juce::String& paramR,
                   juce::Colour curveColour = juce::Colours::cyan);
                   
    ~AdsrVisualizer() override;

    void paint(juce::Graphics& g) override;
    void timerCallback() override;

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

private:
    juce::AudioProcessorValueTreeState& apvts;
    juce::String idA, idD, idS, idR;
    juce::Colour colour;

    float lastA = -1.0f;
    float lastD = -1.0f;
    float lastS = -1.0f;
    float lastR = -1.0f;

    int draggedNode = -1; // 0=A, 1=D/S, 2=R
    float startA, startD, startS, startR;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AdsrVisualizer)
};


class LfoVisualizer : public juce::Component, public juce::Timer
{
public:
    LfoVisualizer(juce::AudioProcessorValueTreeState& apvts,
                  const juce::String& paramFreq,
                  const juce::String& paramAmt,
                  juce::Colour curveColour = juce::Colours::magenta);
                  
    ~LfoVisualizer() override;

    void paint(juce::Graphics& g) override;
    void timerCallback() override;

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

private:
    juce::AudioProcessorValueTreeState& apvts;
    juce::String idFreq, idAmt;
    juce::Colour colour;

    float lastFreq = -1.0f;
    float lastAmt = -1.0f;
    float phase = 0.0f;

    bool isDragging = false;
    juce::Point<int> dragStartPos;
    float startFreq = 0.0f;
    float startAmt = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LfoVisualizer)
};

// ==============================================================================
class OscVisualizer : public juce::Component, public juce::Timer
{
public:
    OscVisualizer(juce::AudioProcessorValueTreeState& apvts,
                  const juce::String& paramWave,
                  const juce::String& paramRatio,
                  juce::Colour curveColour = juce::Colours::cyan,
                  const juce::String& paramFm = "",
                  const juce::String& paramWaveB = "",
                  const juce::String& paramRatioB = "");
                  
    ~OscVisualizer() override;

    void paint(juce::Graphics& g) override;
    void timerCallback() override;

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

private:
    juce::AudioProcessorValueTreeState& apvts;
    juce::String idWave, idRatio;
    juce::String idFm, idWaveB, idRatioB;
    juce::Colour colour;

    float lastWave = -1.0f;
    float lastRatio = -1.0f;
    float lastFm = 0.0f;
    float lastWaveB = 0.0f;
    float lastRatioB = 1.0f;
    float phaseOffset = 0.0f;

    bool isDragging = false;
    float startWave = 0.0f;
    float startRatio = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscVisualizer)
};

// ==============================================================================
class NoiseVisualizer : public juce::Component, public juce::Timer
{
public:
    NoiseVisualizer(juce::AudioProcessorValueTreeState& apvts,
                    const juce::String& paramType);
                  
    ~NoiseVisualizer() override;

    void paint(juce::Graphics& g) override;
    void timerCallback() override;

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

private:
    juce::AudioProcessorValueTreeState& apvts;
    juce::String idType;

    float lastType = -1.0f;

    bool isDragging = false;
    float startType = 0.0f;
    
    std::vector<float> noiseBuffer;
    float phaseOffset = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoiseVisualizer)
};

// ==============================================================================
class FilterVisualizer : public juce::Component, public juce::Timer
{
public:
    FilterVisualizer(juce::AudioProcessorValueTreeState& apvts,
                     const juce::String& paramType,
                     const juce::String& paramRes,
                     const juce::String& paramCutoff);
                  
    ~FilterVisualizer() override;

    void paint(juce::Graphics& g) override;
    void timerCallback() override;

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

private:
    juce::AudioProcessorValueTreeState& apvts;
    juce::String idType, idRes, idCutoff;

    float lastType = -1.0f;
    float lastRes = -1.0f;
    float lastCutoff = -1.0f;

    bool isDragging = false;
    float startType = 0.0f;
    float startRes = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterVisualizer)
};

} // namespace genesynth
