#include "PluginProcessor.h"
#include "PluginEditor.h"

// For now, these are empty placeholder components just to sketch out the layout.
// We will replace them with actual classes in Source/UI/


GeneSynthAudioProcessorEditor::GeneSynthAudioProcessorEditor (GeneSynthAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    juce::LookAndFeel::setDefaultLookAndFeel(&customLookAndFeel);

    oscA = std::make_unique<genesynth::OscASection>(audioProcessor.apvts);
    oscB = std::make_unique<genesynth::OscBSection>(audioProcessor.apvts);
    noise = std::make_unique<genesynth::NoiseSection>(audioProcessor.apvts);
    filter = std::make_unique<genesynth::FilterSectionUI>(audioProcessor.apvts);
    
    modulation = std::make_unique<genesynth::ModulationSectionUI>(audioProcessor.apvts);
    globalEnv = std::make_unique<genesynth::GlobalEnvSection>(audioProcessor.apvts);
    softClipper = std::make_unique<genesynth::SoftClipperSection>(audioProcessor.apvts);
    reverb = std::make_unique<genesynth::ReverbSection>(audioProcessor.apvts);

    addAndMakeVisible(menuButton);
    addAndMakeVisible(saveButton);
    addAndMakeVisible(presetsBox);
    addAndMakeVisible(modeButton);
    addAndMakeVisible(masterVolSlider);

    genesynth::GeneSynthLookAndFeel::setSliderStyle(masterVolSlider, false, juce::Colours::white);

    addAndMakeVisible(oscA.get());
    addAndMakeVisible(oscB.get());
    addAndMakeVisible(noise.get());
    addAndMakeVisible(filter.get());
    
    addAndMakeVisible(modulation.get());
    addAndMakeVisible(globalEnv.get());
    addAndMakeVisible(softClipper.get());
    addAndMakeVisible(reverb.get());

    setSize (1000, 700);
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
    auto bounds = getLocalBounds().reduced(10);
    
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

    // Grid layout
    juce::Grid grid;
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;
    using Px = juce::Grid::Px;

    grid.templateRows = { Track(Fr(3)), Track(Fr(2)), Track(Fr(2)) };
    grid.templateColumns = { Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)) };
    grid.columnGap = Px(10);
    grid.rowGap = Px(10);

    // Generators Row (Row 1)
    grid.items.add(juce::GridItem(oscA.get()).withArea(1, 1, 2, 2));
    grid.items.add(juce::GridItem(oscB.get()).withArea(1, 2, 2, 3));
    grid.items.add(juce::GridItem(noise.get()).withArea(1, 3, 2, 4));
    grid.items.add(juce::GridItem(filter.get()).withArea(1, 4, 2, 5));
    // The 5th column is empty in row 1 for now (or physic could go here later if added)

    // Modulation Row (Row 2)
    // Spans from col 1 to col 4
    grid.items.add(juce::GridItem(modulation.get()).withArea(2, 1, 3, 5));
    // Soft Clipper on the right
    grid.items.add(juce::GridItem(softClipper.get()).withArea(2, 5, 3, 6));

    // Global & Effects Row (Row 3)
    // Global Envelope spans from col 1 to 4
    grid.items.add(juce::GridItem(globalEnv.get()).withArea(3, 1, 4, 5));
    // Reverb on the right
    grid.items.add(juce::GridItem(reverb.get()).withArea(3, 5, 4, 6));

    grid.performLayout(bounds);

    juce::Logger::writeToLog("Editor resized! bounds = " + bounds.toString());
    juce::Logger::writeToLog("oscA bounds = " + oscA->getBounds().toString());
}
