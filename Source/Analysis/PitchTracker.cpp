#include "PitchTracker.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>

namespace genesynth {

float PitchTracker::detectPitchMPM(const float* audioData, int numSamples, double sampleRate) {
    if (numSamples <= 0 || audioData == nullptr) return 0.0f;
    
    std::vector<float> nsdf(numSamples, 0.0f);
    
    // 1. Calculate Normalized Square Difference Function (NSDF)
    // nsdf[tau] = 2 * sum(x[j] * x[j+tau]) / (sum(x[j]^2) + sum(x[j+tau]^2))
    for (int tau = 0; tau < numSamples; ++tau) {
        float acf = 0.0f;
        float m_acf = 0.0f;
        for (int j = 0; j < numSamples - tau; ++j) {
            acf += audioData[j] * audioData[j + tau];
            m_acf += audioData[j] * audioData[j] + audioData[j + tau] * audioData[j + tau];
        }
        if (m_acf == 0.0f) {
            nsdf[tau] = 0.0f;
        } else {
            nsdf[tau] = 2.0f * acf / m_acf;
        }
    }
    
    // 2. Peak Picking
    // Find all the local maxima in the NSDF
    std::vector<int> maxPositions;
    bool positive = false;
    for (int tau = 1; tau < numSamples - 1; ++tau) {
        if (nsdf[tau] > 0.0f) {
            positive = true;
        }
        if (positive && nsdf[tau] < 0.0f) {
            positive = false; // Crossed zero
        }
        
        // It's a local maximum
        if (nsdf[tau] > nsdf[tau - 1] && nsdf[tau] > nsdf[tau + 1]) {
            maxPositions.push_back(tau);
        }
    }
    
    if (maxPositions.empty()) return 0.0f;
    
    // 3. Find the highest peak
    float highestPeak = -1.0f;
    for (int pos : maxPositions) {
        if (nsdf[pos] > highestPeak) {
            highestPeak = nsdf[pos];
        }
    }
    
    // Threshold is usually slightly below the highest peak to avoid octave errors
    // McLeod recommends 0.93 * highestPeak
    float threshold = 0.93f * highestPeak;
    
    // Find the first peak that is above the threshold
    int selectedPeak = -1;
    for (int pos : maxPositions) {
        if (nsdf[pos] >= threshold) {
            selectedPeak = pos;
            break;
        }
    }
    
    if (selectedPeak == -1) return 0.0f;
    
    // 4. Parabolic Interpolation for sub-sample accuracy
    // Using the 3 points around the peak: nsdf[tau-1], nsdf[tau], nsdf[tau+1]
    int tau = selectedPeak;
    if (tau <= 0 || tau >= numSamples - 1) {
        return static_cast<float>(sampleRate / tau);
    }
    
    float alpha = nsdf[tau - 1];
    float beta = nsdf[tau];
    float gamma = nsdf[tau + 1];
    
    // The offset delta is (alpha - gamma) / (2 * (alpha - 2*beta + gamma))
    float denominator = alpha - 2.0f * beta + gamma;
    float delta = 0.0f;
    if (denominator != 0.0f) {
        delta = (alpha - gamma) / (2.0f * denominator);
    }
    
    float exactTau = tau + delta;
    
    // 5. Convert to frequency
    if (exactTau == 0.0f) return 0.0f;
    
    return static_cast<float>(sampleRate / exactTau);
}

} // namespace genesynth
