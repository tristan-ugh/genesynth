#include "AudioResampler.h"

namespace genesynth {

juce::AudioBuffer<float> AudioResampler::repitchToC5(const juce::AudioBuffer<float>& originalBuffer, float originalFundFreq) {
    if (originalFundFreq <= 0.0f || originalBuffer.getNumSamples() == 0) {
        return originalBuffer; // Cannot repitch
    }

    // speedRatio is the step size through the input buffer.
    // If original is 1046 Hz (C6) and target is 523 Hz (C5), speedRatio = 523 / 1046 = 0.5.
    // We will step through input at 0.5x speed, resulting in an output twice as long (pitch halved).
    double speedRatio = C5_FREQUENCY / (double)originalFundFreq;
    
    // Calculate new length
    int numInputSamples = originalBuffer.getNumSamples();
    int numOutputSamples = (int)std::ceil(numInputSamples / speedRatio);
    int numChannels = originalBuffer.getNumChannels();

    juce::AudioBuffer<float> outputBuffer(numChannels, numOutputSamples);
    
    // We use WindowedSincInterpolator for high-quality offline resampling
    for (int ch = 0; ch < numChannels; ++ch) {
        juce::WindowedSincInterpolator interpolator;
        
        const float* inputPtr = originalBuffer.getReadPointer(ch);
        float* outputPtr = outputBuffer.getWritePointer(ch);
        
        // WindowedSincInterpolator consumes input and produces output.
        // The process method returns the number of input samples consumed.
        // We need to keep feeding it until we produce the desired output samples.
        
        int inputSamplesConsumed = 0;
        int outputSamplesGenerated = 0;
        
        while (outputSamplesGenerated < numOutputSamples && inputSamplesConsumed < numInputSamples) {
            int samplesToProduce = std::min(2048, numOutputSamples - outputSamplesGenerated);
            int availableInput = numInputSamples - inputSamplesConsumed;
            
            // process() expects enough input samples. It might need a few extra for its filter kernel.
            // If we are at the end, we might just pad with 0s or break.
            if (availableInput < 100 && outputSamplesGenerated >= numOutputSamples - 100) {
                // Too close to the end for the sinc filter window, just fill the rest with 0
                for (int i = outputSamplesGenerated; i < numOutputSamples; i++) {
                    outputPtr[i] = 0.0f;
                }
                break;
            }
            
            int consumed = interpolator.process(speedRatio, 
                                                inputPtr + inputSamplesConsumed, 
                                                outputPtr + outputSamplesGenerated, 
                                                samplesToProduce, 
                                                availableInput,
                                                0);
            
            inputSamplesConsumed += consumed;
            outputSamplesGenerated += samplesToProduce;
            
            if (consumed == 0) {
                break; // Should not happen, but prevents infinite loop if something goes wrong
            }
        }
    }

    return outputBuffer;
}

juce::AudioBuffer<float> AudioResampler::resampleToTargetRate(const juce::AudioBuffer<float>& originalBuffer, double originalRate, double targetRate) {
    if (originalRate <= 0.0 || targetRate <= 0.0 || originalBuffer.getNumSamples() == 0 || originalRate == targetRate) {
        return originalBuffer; 
    }

    // speedRatio is inputRate / outputRate
    // e.g. 44100 -> 22050: speedRatio = 2.0 (we step through input twice as fast, resulting in half the length)
    double speedRatio = originalRate / targetRate;
    
    int numInputSamples = originalBuffer.getNumSamples();
    int numOutputSamples = (int)std::ceil(numInputSamples / speedRatio);
    int numChannels = originalBuffer.getNumChannels();

    juce::AudioBuffer<float> outputBuffer(numChannels, numOutputSamples);
    
    for (int ch = 0; ch < numChannels; ++ch) {
        juce::WindowedSincInterpolator interpolator;
        
        const float* inputPtr = originalBuffer.getReadPointer(ch);
        float* outputPtr = outputBuffer.getWritePointer(ch);
        
        int inputSamplesConsumed = 0;
        int outputSamplesGenerated = 0;
        
        while (outputSamplesGenerated < numOutputSamples && inputSamplesConsumed < numInputSamples) {
            int samplesToProduce = std::min(2048, numOutputSamples - outputSamplesGenerated);
            int availableInput = numInputSamples - inputSamplesConsumed;
            
            if (availableInput < 100 && outputSamplesGenerated >= numOutputSamples - 100) {
                for (int i = outputSamplesGenerated; i < numOutputSamples; i++) {
                    outputPtr[i] = 0.0f;
                }
                break;
            }
            
            int consumed = interpolator.process(speedRatio, 
                                                inputPtr + inputSamplesConsumed, 
                                                outputPtr + outputSamplesGenerated, 
                                                samplesToProduce, 
                                                availableInput,
                                                0);
            
            inputSamplesConsumed += consumed;
            outputSamplesGenerated += samplesToProduce;
            
            if (consumed == 0) {
                break; 
            }
        }
    }

    return outputBuffer;
}

} // namespace genesynth
