#include "GeneSynthAPI.h"
#include "../PluginProcessor.h"
#include "../Analysis/FeatureExtractor.h"
#include <memory>
#include <iostream>

struct GeneSynthContext {
    std::unique_ptr<GeneSynthAudioProcessor> processor;
    genesynth::FeatureExtractor featureExtractor;
    double sampleRate;
    juce::AudioBuffer<float> renderBuffer;
    juce::MidiBuffer midiBuffer;
    std::vector<juce::String> paramIds;
};

extern "C" {

void* GeneSynth_Create(double sampleRate) {
    // Ensure JUCE's MessageManager is initialized if needed
    juce::MessageManager::getInstance();
    
    auto* ctx = new GeneSynthContext();
    ctx->sampleRate = sampleRate;
    ctx->processor = std::make_unique<GeneSynthAudioProcessor>();
    ctx->processor->prepareToPlay(sampleRate, 512);
    ctx->renderBuffer.setSize(2, (int)sampleRate); // 1 second of stereo audio
    
    // Cache the parameter IDs
    for (int i = 0; i < ctx->processor->getParameters().size(); ++i) {
        if (auto* p = dynamic_cast<juce::AudioProcessorParameterWithID*>(ctx->processor->getParameters()[i])) {
            ctx->paramIds.push_back(p->paramID);
        }
    }
    
    return ctx;
}

void GeneSynth_Destroy(void* instance) {
    if (instance != nullptr) {
        auto* ctx = static_cast<GeneSynthContext*>(instance);
        delete ctx;
    }
}

int GeneSynth_GetNumParameters(void* instance) {
    if (instance == nullptr) return 0;
    auto* ctx = static_cast<GeneSynthContext*>(instance);
    return (int)ctx->paramIds.size();
}

const char* GeneSynth_GetParameterId(void* instance, int index) {
    if (instance == nullptr) return nullptr;
    auto* ctx = static_cast<GeneSynthContext*>(instance);
    if (index >= 0 && index < ctx->paramIds.size()) {
        return ctx->paramIds[(size_t)index].toRawUTF8();
    }
    return nullptr;
}

void GeneSynth_RenderFeatures(void* instance, const float* params, float* outSpectrogram) {
    if (instance == nullptr) return;
    auto* ctx = static_cast<GeneSynthContext*>(instance);
    
    // 1. Update Parameters
    for (size_t i = 0; i < ctx->paramIds.size(); ++i) {
        if (auto* p = ctx->processor->apvts.getParameter(ctx->paramIds[i])) {
            p->setValueNotifyingHost(params[i]);
        }
    }
    
    // 2. Clear buffers and send Note On
    ctx->renderBuffer.clear();
    ctx->midiBuffer.clear();
    ctx->midiBuffer.addEvent(juce::MidiMessage::noteOn(1, 72, (juce::uint8)100), 0);
    
    // 3. Process Audio (Render 1 second)
    int numSamples = ctx->renderBuffer.getNumSamples();
    int blockSize = 512;
    int sampleIndex = 0;
    
    while (sampleIndex < numSamples) {
        int numThisTime = std::min(blockSize, numSamples - sampleIndex);
        juce::AudioBuffer<float> subBuffer(ctx->renderBuffer.getArrayOfWritePointers(), 2, sampleIndex, numThisTime);
        
        ctx->processor->processBlock(subBuffer, ctx->midiBuffer);
        
        ctx->midiBuffer.clear(); // Only send Note On in the first block
        sampleIndex += numThisTime;
        
        // After 500ms, send Note Off to let release ring
        if (sampleIndex >= numSamples / 2 && sampleIndex - numThisTime < numSamples / 2) {
            ctx->midiBuffer.addEvent(juce::MidiMessage::noteOff(1, 72), 0);
        }
    }
    
    // 4. Extract Features
    auto features = ctx->featureExtractor.extractFeatures(ctx->renderBuffer, ctx->sampleRate);
    
    // 5. Copy to outSpectrogram
    if (features.numFrames == 86 && outSpectrogram != nullptr) {
        std::copy(features.logMelSpectrogram.begin(), features.logMelSpectrogram.end(), outSpectrogram);
    }
}

} // extern "C"
