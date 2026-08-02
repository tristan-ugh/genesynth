#pragma once

#include <JuceHeader.h>
#include "../SynthComponents.h"
#include "../../Analysis/InferenceEngine.h"

namespace genesynth {

// ==============================================================================
class MorphPadComponent : public juce::Component, public juce::Timer
{
public:
    MorphPadComponent();
    ~MorphPadComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

    std::function<void(float, float)> onNodeMoved;

private:
    struct Node {
        juce::Point<float> pos;
        bool isDragging = false;
        juce::Colour color;
    };
    std::vector<Node> nodes;
    int draggedNodeIndex = -1;
    float animPhase = 0.0f;
    juce::Image innerShadowImage;
    juce::Image outerShadowImage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MorphPadComponent)
};

// ==============================================================================
class MorphBottomBar : public juce::Component,
                       public juce::ChangeListener,
                       public juce::FileDragAndDropTarget
{
public:
    MorphBottomBar(juce::AudioProcessorValueTreeState& apvts, genesynth::InferenceEngine* inferenceEngine);
    ~MorphBottomBar() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // ChangeListener
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    // FileDragAndDropTarget
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    void loadFile(const juce::File& file);

private:
    juce::TextButton importBtn{"imp."};
    juce::TextButton generateBtn{"generate"};
    
    LabeledSlider spiceKnob;
    LabeledSlider brightKnob;
    LabeledSlider timeKnob;
    
    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache{5};
    juce::AudioThumbnail thumbnail;
    std::unique_ptr<juce::FileChooser> fileChooser;
    genesynth::InferenceEngine* inferenceEngine = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MorphBottomBar)
};

// ==============================================================================
class MorphView : public juce::Component
{
public:
    MorphView(juce::AudioProcessorValueTreeState& apvts, genesynth::InferenceEngine* inferenceEngine);
    ~MorphView() override = default;

    void resized() override;

private:
    MorphPadComponent pad;
    MorphBottomBar bottomBar;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MorphView)
};

} // namespace genesynth
