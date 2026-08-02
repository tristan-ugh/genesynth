#include "FeatureExtractor.h"
#include <cmath>
#include <algorithm>

namespace genesynth {

FeatureExtractor::FeatureExtractor() 
    : fft(FFT_ORDER), window(FFT_SIZE, juce::dsp::WindowingFunction<float>::hann) {
    createDCTMatrix();
}

void FeatureExtractor::createMelFilterbank(double sampleRate) {
    if (sampleRate == cachedSampleRate && !melFilterbank.empty()) return;
    
    cachedSampleRate = sampleRate;
    int numBins = FFT_SIZE / 2 + 1;
    melFilterbank.assign(NUM_MELS, std::vector<float>(numBins, 0.0f));
    
    float minHz = 0.0f;
    float maxHz = (float)(sampleRate / 2.0);
    
    float minMel = hzToMel(minHz);
    float maxMel = hzToMel(maxHz);
    
    std::vector<float> melPoints(NUM_MELS + 2);
    for (int i = 0; i < NUM_MELS + 2; ++i) {
        melPoints[i] = minMel + i * (maxMel - minMel) / (NUM_MELS + 1);
    }
    
    std::vector<float> hzPoints(NUM_MELS + 2);
    std::vector<int> binPoints(NUM_MELS + 2);
    for (int i = 0; i < NUM_MELS + 2; ++i) {
        hzPoints[i] = melToHz(melPoints[i]);
        binPoints[i] = (int)std::floor((FFT_SIZE + 1) * hzPoints[i] / sampleRate);
        if (binPoints[i] >= numBins) binPoints[i] = numBins - 1;
    }
    
    for (int m = 1; m <= NUM_MELS; ++m) {
        int leftBin = binPoints[m - 1];
        int centerBin = binPoints[m];
        int rightBin = binPoints[m + 1];
        
        for (int k = leftBin; k < centerBin; ++k) {
            melFilterbank[m - 1][k] = (float)(k - leftBin) / (float)std::max(1, centerBin - leftBin);
        }
        for (int k = centerBin; k < rightBin; ++k) {
            melFilterbank[m - 1][k] = (float)(rightBin - k) / (float)std::max(1, rightBin - centerBin);
        }
    }
}

void FeatureExtractor::createDCTMatrix() {
    dctMatrix.assign(NUM_MFCCS, std::vector<float>(NUM_MELS, 0.0f));
    float factor = std::sqrt(2.0f / NUM_MELS);
    for (int i = 0; i < NUM_MFCCS; ++i) {
        float ci = (i == 0) ? std::sqrt(1.0f / NUM_MELS) : factor;
        for (int j = 0; j < NUM_MELS; ++j) {
            dctMatrix[i][j] = ci * std::cos(juce::MathConstants<float>::pi * i * (j + 0.5f) / NUM_MELS);
        }
    }
}

FeatureData FeatureExtractor::extractFeatures(const juce::AudioBuffer<float>& audioBuffer, double sampleRate) {
    FeatureData data;
    data.numMels = NUM_MELS;
    data.numMfccs = NUM_MFCCS;
    
    if (audioBuffer.getNumSamples() < FFT_SIZE) return data;
    
    createMelFilterbank(sampleRate);
    
    const float* input = audioBuffer.getReadPointer(0);
    int numSamples = audioBuffer.getNumSamples();
    
    data.numFrames = 1 + (numSamples - FFT_SIZE) / HOP_LENGTH;
    
    data.logMelSpectrogram.reserve(data.numFrames * NUM_MELS);
    data.mfccs.reserve(data.numFrames * NUM_MFCCS);
    
    std::vector<float> fftBuffer(FFT_SIZE * 2, 0.0f);
    
    for (int frame = 0; frame < data.numFrames; ++frame) {
        int startSample = frame * HOP_LENGTH;
        
        // Copy to FFT buffer and apply window
        for (int i = 0; i < FFT_SIZE; ++i) {
            fftBuffer[i] = input[startSample + i];
        }
        window.multiplyWithWindowingTable(fftBuffer.data(), FFT_SIZE);
        
        // Pad the rest of the array with 0s for complex FFT processing
        std::fill(fftBuffer.begin() + FFT_SIZE, fftBuffer.end(), 0.0f);
        
        // Perform Forward FFT
        fft.performFrequencyOnlyForwardTransform(fftBuffer.data());
        
        std::vector<float> melEnergies(NUM_MELS, 0.0f);
        
        // Apply Mel Filterbank
        for (int m = 0; m < NUM_MELS; ++m) {
            float energy = 0.0f;
            for (int k = 0; k <= FFT_SIZE / 2; ++k) {
                // Slaney-style mel filterbanks usually use power spectrum
                float mag = std::max(0.0f, fftBuffer[k]); 
                energy += mag * mag * melFilterbank[m][k];
            }
            
            // Log-Mel
            float logEnergy = 10.0f * std::log10(std::max(1e-10f, energy));
            melEnergies[m] = logEnergy;
            data.logMelSpectrogram.push_back(logEnergy);
        }
        
        // Compute MFCCs
        for (int i = 0; i < NUM_MFCCS; ++i) {
            float mfcc = 0.0f;
            for (int j = 0; j < NUM_MELS; ++j) {
                mfcc += melEnergies[j] * dctMatrix[i][j];
            }
            data.mfccs.push_back(mfcc);
        }
    }
    
    return data;
}

} // namespace genesynth
