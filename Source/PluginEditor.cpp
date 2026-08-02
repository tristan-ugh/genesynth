#include "PluginProcessor.h"
#include "PluginEditor.h"

// For now, these are empty placeholder components just to sketch out the layout.
// We will replace them with actual classes in Source/UI/


GeneSynthAudioProcessorEditor::GeneSynthAudioProcessorEditor (GeneSynthAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    juce::LookAndFeel::setDefaultLookAndFeel(&customLookAndFeel);

    synthView = std::make_unique<genesynth::SynthView>(audioProcessor.apvts, audioProcessor.keyboardState);
    morphView = std::make_unique<genesynth::MorphView>(audioProcessor.apvts, audioProcessor.inferenceEngine.get());

    addAndMakeVisible(synthView.get());
    addChildComponent(morphView.get()); // Hidden by default

    addAndMakeVisible(menuButton);
    addAndMakeVisible(saveButton);
    addAndMakeVisible(presetsBox);
    addAndMakeVisible(modeButton);
    addAndMakeVisible(masterVolSlider);

    genesynth::GeneSynthLookAndFeel::setSliderStyle(masterVolSlider, false, juce::Colours::white);

    modeButton.onClick = [this] { toggleMode(); };

    setSize (1000, 800);
}

GeneSynthAudioProcessorEditor::~GeneSynthAudioProcessorEditor()
{
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

void GeneSynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void GeneSynthAudioProcessorEditor::resized()
{
    auto fullBounds = getLocalBounds();
    auto bounds = fullBounds.reduced(10);
    
    // Top Bar
    auto topBar = bounds.removeFromTop(40);
    menuButton.setBounds(topBar.removeFromLeft(40));
    topBar.removeFromLeft(10);
    saveButton.setBounds(topBar.removeFromLeft(60));
    topBar.removeFromLeft(20);
    
    // Vol and Mode on the right
    masterVolSlider.setBounds(topBar.removeFromRight(40));
    topBar.removeFromRight(10);
    modeButton.setBounds(topBar.removeFromRight(60));
    topBar.removeFromRight(20);
    
    presetsBox.setBounds(topBar.reduced(0, 5));

    bounds.removeFromTop(20);

    if (isMorphMode) {
        morphView->setBounds(bounds);
    } else {
        // Give synthView the full bounds so it can draw massive neumorphic shadows 
        // that bleed into the window margins without clipping!
        synthView->setBounds(fullBounds);
    }
}

void GeneSynthAudioProcessorEditor::toggleMode()
{
    isMorphMode = !isMorphMode;
    synthView->setVisible(!isMorphMode);
    morphView->setVisible(isMorphMode);
    resized();
}
