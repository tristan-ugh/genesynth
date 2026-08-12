#include "Source/Analysis/PitchTracker.h"
#include <iostream>
#include <cmath>
#include <vector>

int main() {
    double sampleRate = 44100.0;
    int numSamples = 2048;
    std::vector<float> buffer(numSamples, 0.0f);
    
    // Test 1: Sine wave at 440 Hz
    float freq = 440.0f;
    for (int i = 0; i < numSamples; ++i) {
        buffer[i] = std::sin(2.0f * 3.14159265359f * freq * i / sampleRate);
    }
    
    float detected = genesynth::PitchTracker::detectPitchMPM(buffer.data(), numSamples, sampleRate);
    std::cout << "Test 1 (Sine 440Hz): Detected " << detected << " Hz\n";
    
    // Test 2: Square wave at 110 Hz (rich in odd harmonics)
    freq = 110.0f;
    for (int i = 0; i < numSamples; ++i) {
        float t = fmod(freq * i / sampleRate, 1.0f);
        buffer[i] = (t < 0.5f) ? 1.0f : -1.0f;
    }
    
    detected = genesynth::PitchTracker::detectPitchMPM(buffer.data(), numSamples, sampleRate);
    std::cout << "Test 2 (Square 110Hz): Detected " << detected << " Hz\n";
    
    return 0;
}
