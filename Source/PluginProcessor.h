#pragma once

#include <JuceHeader.h>
#include "Framework/aBlock/aContainer/SynthEngine/SynthEngine.h"
#include "Analysis/InferenceEngine.h"
#include "Parameters/SynthParameter.h"
#include <vector>
#include <memory>

class GeneSynthAudioProcessor : public juce::AudioProcessor
{
public:
    GeneSynthAudioProcessor();
    ~GeneSynthAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    juce::MidiKeyboardState keyboardState;
    
    // IA Engine
    std::unique_ptr<genesynth::InferenceEngine> inferenceEngine;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    // Le Propriétaire Unique des paramètres OOP
    std::vector<std::unique_ptr<genesynth::SynthParameter>> allParams;
    
    // Moteur principal
    genesynth::SynthEngine engine;
    
    // Helper pour récupérer un paramètre avec le bon type depuis allParams
    template <typename T>
    T* getParamAs(genesynth::ParamID id) {
        for (auto& p : allParams) {
            if (p->getId() == id) {
                return dynamic_cast<T*>(p.get());
            }
        }
        jassertfalse; // Parameter not found or bad cast
        return nullptr;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GeneSynthAudioProcessor)
};
