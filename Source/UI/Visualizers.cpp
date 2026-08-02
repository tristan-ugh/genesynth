#include "Visualizers.h"

namespace genesynth {

// ==============================================================================
AdsrVisualizer::AdsrVisualizer(juce::AudioProcessorValueTreeState& apvts,
                               const juce::String& paramA,
                               const juce::String& paramD,
                               const juce::String& paramS,
                               const juce::String& paramR,
                               juce::Colour curveColour)
    : apvts(apvts), idA(paramA), idD(paramD), idS(paramS), idR(paramR), colour(curveColour)
{
    startTimerHz(30);
}

AdsrVisualizer::~AdsrVisualizer()
{
    stopTimer();
}

void AdsrVisualizer::timerCallback()
{
    float a = *apvts.getRawParameterValue(idA);
    float d = *apvts.getRawParameterValue(idD);
    float s = *apvts.getRawParameterValue(idS);
    float r = *apvts.getRawParameterValue(idR);

    if (a != lastA || d != lastD || s != lastS || r != lastR) {
        lastA = a; lastD = d; lastS = s; lastR = r;
        repaint();
    }
}

void AdsrVisualizer::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Inner shadow for the screen background
    g.setColour(juce::Colour(0xff060606)); // Almost black inner screen
    g.fillRoundedRectangle(bounds, 5.0f);
    
    juce::ColourGradient innerShadow(juce::Colours::black.withAlpha(0.8f), 0, bounds.getY(),
                                     juce::Colours::transparentBlack, 0, bounds.getY() + 15.0f, false);
    g.setGradientFill(innerShadow);
    g.fillRoundedRectangle(bounds, 5.0f);
    
    g.setColour(juce::Colours::black.withAlpha(0.6f));
    g.drawRoundedRectangle(bounds.reduced(1.0f), 5.0f, 1.0f);

    bounds.reduce(10.0f, 10.0f);

    float a = std::max(0.01f, lastA);
    float d = std::max(0.01f, lastD);
    float s = lastS;
    float r = std::max(0.01f, lastR);

    float w = bounds.getWidth();
    float h = bounds.getHeight();
    float bottom = bounds.getBottom();
    float left = bounds.getX();

    float maxSegment = w * 0.3f;
    
    float xA = left + (a * maxSegment);
    float xD = xA + (d * maxSegment);
    float xR = left + w;
    float xS = xR - (r * maxSegment);

    float yPeak = bounds.getY();
    float ySus = bottom - (s * s) * h; // Sustain height is squared to match DSP n^2 curve

    juce::Path p;
    p.startNewSubPath(left, bottom);
    
    int steps = 20;
    // Attack
    for (int i = 1; i <= steps; ++i) {
        float t = (float)i / steps;
        float val = t * t;
        float x = left + t * (xA - left);
        float y = bottom - val * (bottom - yPeak);
        p.lineTo(x, y);
    }
    // Decay
    for (int i = 1; i <= steps; ++i) {
        float t = (float)i / steps;
        float linearVal = 1.0f - t * (1.0f - s);
        float val = linearVal * linearVal;
        float x = xA + t * (xD - xA);
        float y = bottom - val * (bottom - yPeak);
        p.lineTo(x, y);
    }
    // Sustain
    p.lineTo(xS, ySus);
    
    // Release
    for (int i = 1; i <= steps; ++i) {
        float t = (float)i / steps;
        float linearVal = s * (1.0f - t);
        float val = linearVal * linearVal;
        float x = xS + t * (xR - xS);
        float y = bottom - val * (bottom - yPeak);
        p.lineTo(x, y);
    }


    // Vertical fill
    juce::ColourGradient grad(colour.withAlpha(0.1f), 0, yPeak, colour.withAlpha(0.0f), 0, bottom, false);
    juce::Path filledPath = p;
    filledPath.lineTo(left, bottom);
    filledPath.closeSubPath();
    g.setGradientFill(grad);
    g.fillPath(filledPath);

    // Real Gaussian Glow
    juce::Path strokedPath;
    juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded).createStrokedPath(strokedPath, p);
    juce::DropShadow glow(colour.withAlpha(0.8f), 12, juce::Point<int>(0, 0));
    glow.drawForPath(g, strokedPath);

    // Original thin line
    g.setColour(colour);
    g.strokePath(p, juce::PathStrokeType(1.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    
    // Draw glowing nodes
    auto drawGlowingNode = [&](float nx, float ny) {
        juce::Colour nodeBaseColor = juce::Colour::fromHSV(0.33f, 0.08f, 1.0f, 1.0f); // Desaturated bright green
        
        // Ambient node halo
        float ambientRadius = bounds.getWidth() * 0.4f;
        juce::Colour ambientColor = nodeBaseColor.withAlpha(0.06f);
        juce::ColourGradient ambientGrad(ambientColor, nx, ny,
                                         juce::Colours::transparentBlack, nx + ambientRadius, ny, true);
        g.setGradientFill(ambientGrad);
        g.fillEllipse(nx - ambientRadius, ny - ambientRadius, ambientRadius * 2.0f, ambientRadius * 2.0f);

        juce::Path circle;
        circle.addEllipse(nx - 2.5f, ny - 2.5f, 5.0f, 5.0f);
        
        juce::DropShadow nodeGlow(nodeBaseColor.withAlpha(0.8f), 8, juce::Point<int>(0, 0));
        nodeGlow.drawForPath(g, circle);
        
        g.setColour(nodeBaseColor); 
        g.fillPath(circle);
    };

    drawGlowingNode(xA, yPeak);
    drawGlowingNode(xD, ySus);
    drawGlowingNode(xS, ySus);
}

void AdsrVisualizer::mouseDown(const juce::MouseEvent& e) {
    float a = std::max(0.01f, lastA);
    float d = std::max(0.01f, lastD);
    float s = lastS;
    float r = std::max(0.01f, lastR);

    auto bounds = getLocalBounds().toFloat().reduced(10.0f);
    float w = bounds.getWidth();
    float h = bounds.getHeight();
    float bottom = bounds.getBottom();
    float left = bounds.getX();

    float maxSegment = w * 0.3f;

    float xA = left + (a * maxSegment);
    float xD = xA + (d * maxSegment);
    float xR = left + w;
    float xS = xR - (r * maxSegment);
    float yPeak = bounds.getY();
    float ySus = bottom - (s * s) * h;

    juce::Point<float> pA(xA, yPeak);
    juce::Point<float> pD(xD, ySus);
    juce::Point<float> pS(xS, ySus);

    float distA = e.position.getDistanceFrom(pA);
    float distD = e.position.getDistanceFrom(pD);
    float distS = e.position.getDistanceFrom(pS);

    draggedNode = -1;
    float threshold = 20.0f;
    if (distA < threshold && distA < distD && distA < distS) draggedNode = 0;
    else if (distD < threshold && distD < distS) draggedNode = 1;
    else if (distS < threshold) draggedNode = 2;

    startA = lastA; startD = lastD; startS = lastS; startR = lastR;
}

void AdsrVisualizer::mouseDrag(const juce::MouseEvent& e) {
    if (draggedNode == -1) return;
    auto bounds = getLocalBounds().toFloat().reduced(10.0f);
    
    // Scale dx properly. 30% of width represents the full 0..1 range!
    float maxSegment = bounds.getWidth() * 0.3f;
    float dx = e.getDistanceFromDragStartX() / maxSegment;
    float dy = -e.getDistanceFromDragStartY() / bounds.getHeight();

    if (draggedNode == 0) { // A
        float newVal = juce::jlimit(0.0f, 1.0f, startA + dx);
        apvts.getParameter(idA)->setValueNotifyingHost(newVal);
    }
    else if (draggedNode == 1) { // D/S
        float newD = juce::jlimit(0.0f, 1.0f, startD + dx);
        float currentVisualHeight = startS * startS;
        float newVisualHeight = juce::jlimit(0.0f, 1.0f, currentVisualHeight + dy);
        float newS = std::sqrt(newVisualHeight);
        apvts.getParameter(idD)->setValueNotifyingHost(newD);
        apvts.getParameter(idS)->setValueNotifyingHost(newS);
    }
    else if (draggedNode == 2) { // R/S
        float newR = juce::jlimit(0.0f, 1.0f, startR - dx);
        float currentVisualHeight = startS * startS;
        float newVisualHeight = juce::jlimit(0.0f, 1.0f, currentVisualHeight + dy);
        float newS = std::sqrt(newVisualHeight);
        apvts.getParameter(idR)->setValueNotifyingHost(newR);
        apvts.getParameter(idS)->setValueNotifyingHost(newS);
    }
}

void AdsrVisualizer::mouseUp(const juce::MouseEvent& e) {
    draggedNode = -1;
}

// ==============================================================================
LfoVisualizer::LfoVisualizer(juce::AudioProcessorValueTreeState& apvts,
                             const juce::String& paramFreq,
                             const juce::String& paramAmt,
                             juce::Colour curveColour)
    : apvts(apvts), idFreq(paramFreq), idAmt(paramAmt), colour(curveColour)
{
    startTimerHz(30);
}

LfoVisualizer::~LfoVisualizer()
{
    stopTimer();
}

void LfoVisualizer::timerCallback()
{
    float freq = *apvts.getRawParameterValue(idFreq);
    float amt = *apvts.getRawParameterValue(idAmt);

    // Update phase for animation based on frequency (mapped visually)
    // frequency parameter is usually 0.0 to 1.0, map to reasonable visual speed
    float visualSpeed = 0.05f + (freq * 0.4f); 
    phase += visualSpeed;
    if (phase > juce::MathConstants<float>::twoPi * 1000.0f) {
        phase = 0.0f; // avoid overflow
    }

    if (freq != lastFreq || amt != lastAmt) {
        lastFreq = freq; lastAmt = amt;
    }
    
    // Always repaint to animate
    repaint();
}

void LfoVisualizer::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Inner shadow for the screen background
    g.setColour(juce::Colour(0xff060606)); // Almost black inner screen
    g.fillRoundedRectangle(bounds, 5.0f);
    
    juce::ColourGradient innerShadow(juce::Colours::black.withAlpha(0.8f), 0, bounds.getY(),
                                     juce::Colours::transparentBlack, 0, bounds.getY() + 15.0f, false);
    g.setGradientFill(innerShadow);
    g.fillRoundedRectangle(bounds, 5.0f);
    
    g.setColour(juce::Colours::black.withAlpha(0.6f));
    g.drawRoundedRectangle(bounds.reduced(1.0f), 5.0f, 1.0f);

    bounds.reduce(5.0f, 10.0f); // padding

    float amt = lastAmt;
    float w = bounds.getWidth();
    float h = bounds.getHeight();
    float midY = bounds.getCentreY();
    float left = bounds.getX();

    juce::Path p;
    bool first = true;
    
    // Draw the sine wave
    // The number of cycles visible depends on the frequency parameter
    float numCycles = 1.0f + lastFreq * 5.0f;
    
    for (float x = 0; x <= w; x += 2.0f) {
        // x ratio
        float nx = x / w;
        // calculate sine
        float y = std::sin(nx * juce::MathConstants<float>::twoPi * numCycles - phase);
        
        // Scale by amount and height
        float py = midY - (y * amt * (h / 2.0f));
        
        if (first) {
            p.startNewSubPath(left + x, py);
            first = false;
        } else {
            p.lineTo(left + x, py);
        }
    }

    // Fill underneath the wave to the center line
    juce::Path filledPath = p;
    filledPath.lineTo(left + w, midY);
    filledPath.lineTo(left, midY);
    filledPath.closeSubPath();
    
    // Ambient diffuse halo
    float ambientRadius = bounds.getWidth() * 0.6f;
    juce::Colour ambientColor = colour.withMultipliedSaturation(1.2f).withAlpha(0.05f);
    juce::ColourGradient ambientGrad(ambientColor, bounds.getCentreX(), bounds.getCentreY(),
                                     juce::Colours::transparentBlack, bounds.getCentreX() + ambientRadius, bounds.getCentreY(), true);
    g.setGradientFill(ambientGrad);
    g.fillEllipse(bounds.getCentreX() - ambientRadius, bounds.getCentreY() - ambientRadius, ambientRadius * 2.0f, ambientRadius * 2.0f);

    // Vertical fill
    juce::ColourGradient grad(colour.withAlpha(0.1f), 0, bounds.getY(), colour.withAlpha(0.0f), 0, midY, false);
    g.setGradientFill(grad);
    g.fillPath(filledPath);

    // Real Gaussian Glow
    juce::Path strokedPath;
    juce::PathStrokeType(2.0f).createStrokedPath(strokedPath, p);
    juce::DropShadow glow(colour.withAlpha(0.8f), 12, juce::Point<int>(0, 0));
    glow.drawForPath(g, strokedPath);

    // Original thin line
    g.setColour(colour);
    g.strokePath(p, juce::PathStrokeType(1.0f));
}

void LfoVisualizer::mouseDown(const juce::MouseEvent& e) {
    isDragging = true;
    startFreq = lastFreq;
    startAmt = lastAmt;
}

void LfoVisualizer::mouseDrag(const juce::MouseEvent& e) {
    if (!isDragging) return;
    auto bounds = getLocalBounds().toFloat().reduced(5.0f, 10.0f);
    float dx = e.getDistanceFromDragStartX() / 200.0f;
    float dy = -e.getDistanceFromDragStartY() / bounds.getHeight();

    float newFreq = juce::jlimit(0.0f, 1.0f, startFreq + dx);
    float newAmt = juce::jlimit(0.0f, 1.0f, startAmt + dy);
    
    apvts.getParameter(idFreq)->setValueNotifyingHost(newFreq);
    apvts.getParameter(idAmt)->setValueNotifyingHost(newAmt);
}

void LfoVisualizer::mouseUp(const juce::MouseEvent& e) {
    isDragging = false;
}

// ==============================================================================
OscVisualizer::OscVisualizer(juce::AudioProcessorValueTreeState& apvts,
                             const juce::String& paramWave,
                             const juce::String& paramRatio,
                             juce::Colour curveColour,
                             const juce::String& paramFm,
                             const juce::String& paramWaveB,
                             const juce::String& paramRatioB)
    : apvts(apvts), idWave(paramWave), idRatio(paramRatio), 
      idFm(paramFm), idWaveB(paramWaveB), idRatioB(paramRatioB), colour(curveColour)
{
    startTimerHz(60); // 60 FPS for buttery smooth animation
}

OscVisualizer::~OscVisualizer()
{
    stopTimer();
}

void OscVisualizer::timerCallback()
{
    lastWave = *apvts.getRawParameterValue(idWave);
    lastRatio = *apvts.getRawParameterValue(idRatio);
    
    if (idFm.isNotEmpty()) {
        lastFm = *apvts.getRawParameterValue(idFm);
        lastWaveB = *apvts.getRawParameterValue(idWaveB);
        lastRatioB = *apvts.getRawParameterValue(idRatioB);
    }

    // Scroll speed
    phaseOffset += 0.0075f; 
    if (phaseOffset >= 1.0f) phaseOffset -= 1.0f;
    
    repaint();
}

void OscVisualizer::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Inner shadow for the screen background
    g.setColour(juce::Colour(0xff060606));
    g.fillRoundedRectangle(bounds, 5.0f);
    
    juce::ColourGradient innerShadow(juce::Colours::black.withAlpha(0.8f), 0, bounds.getY(),
                                     juce::Colours::transparentBlack, 0, bounds.getY() + 15.0f, false);
    g.setGradientFill(innerShadow);
    g.fillRoundedRectangle(bounds, 5.0f);
    
    g.setColour(juce::Colours::black.withAlpha(0.6f));
    g.drawRoundedRectangle(bounds.reduced(1.0f), 5.0f, 1.0f);

    bounds.reduce(10.0f, 10.0f);
    float w = bounds.getWidth();
    float h = bounds.getHeight();
    float cy = bounds.getCentreY();
    float left = bounds.getX();

    juce::Path p;
    float waveType = std::max(0.0f, lastWave);
    float wf = waveType * 3.0f;
    int w1 = std::clamp((int)std::floor(wf), 0, 3);
    int w2 = std::clamp(w1 + 1, 0, 3);
    float mix = wf - w1;

    float numCycles = std::max(0.1f, lastRatio);
    bool first = true;
    
    auto getColor = [](int type) -> juce::Colour {
        if (type == 0) return juce::Colours::cyan;
        if (type == 1) return juce::Colours::lime;
        if (type == 2) return juce::Colours::yellow;
        return juce::Colours::magenta;
    };
    
    juce::Colour c1 = getColor(w1);
    juce::Colour currentColour = c1;
    if (mix > 0.001f && w1 != w2) {
        juce::Colour c2 = getColor(w2);
        currentColour = c1.interpolatedWith(c2, mix);
    }
    
    currentColour = currentColour.withMultipliedSaturation(0.3f);

    auto getWavePoint = [](int type, float phaseMod) -> float {
        if (type == 0) return std::sin(phaseMod * juce::MathConstants<float>::twoPi);
        if (type == 1) {
            float s = phaseMod + 0.25f;
            if (s >= 1.0f) s -= 1.0f;
            return 1.0f - 4.0f * std::abs(s - 0.5f);
        }
        if (type == 2) return phaseMod < 0.5f ? 1.0f : -1.0f;
        return 1.0f - 2.0f * phaseMod;
    };

    // Use sub-pixel sampling (x += 0.2f) to eliminate horizontal/vertical aliasing and trembling
    float step = 0.2f;
    float dtBase = (step / w) * numCycles;
    float integratedPhase = -phaseOffset;
    
    for (float x = 0; x <= w; x += step) {
        float displayPhase = integratedPhase;
        
        // Apply TZFM if this is Osc A
        if (idFm.isNotEmpty() && lastFm > 0.001f) {
            float phaseB = (x / w) * numCycles * (lastRatioB / std::max(0.01f, lastRatio)) - phaseOffset * (lastRatioB / std::max(0.01f, lastRatio));
            phaseB -= std::floor(phaseB);
            
            float waveBType = std::max(0.0f, lastWaveB);
            float wfB = waveBType * 3.0f;
            int w1B = std::clamp((int)std::floor(wfB), 0, 3);
            int w2B = std::clamp(w1B + 1, 0, 3);
            float mixB = wfB - w1B;
            
            float yB = getWavePoint(w1B, phaseB);
            if (mixB > 0.001f && w1B != w2B) {
                yB = yB * (1.0f - mixB) + getWavePoint(w2B, phaseB) * mixB;
            }
            
            float fmAmount = lastFm * 4.0f; // matches the * 4.0f in OscA.h
            float modulatedDt = dtBase * (1.0f + fmAmount * yB);
            integratedPhase += modulatedDt;
        } else {
            integratedPhase += dtBase;
        }

        displayPhase -= std::floor(displayPhase);
        if (displayPhase < 0.0f) displayPhase += 1.0f; // Ensure positive in case phase is negative
        
        float y = getWavePoint(w1, displayPhase);
        if (mix > 0.001f && w1 != w2) {
            y = y * (1.0f - mix) + getWavePoint(w2, displayPhase) * mix;
        }
        
        float py = cy - y * (h * 0.4f);
        if (first) {
            p.startNewSubPath(left + x, py);
            first = false;
        } else {
            p.lineTo(left + x, py);
        }
    }

    // Real Gaussian Glow
    juce::Path strokedPath;
    juce::PathStrokeType(2.0f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded).createStrokedPath(strokedPath, p);
    juce::DropShadow glow(currentColour.withAlpha(0.8f), 12, juce::Point<int>(0, 0));
    glow.drawForPath(g, strokedPath);

    // Original thin line
    g.setColour(currentColour);
    g.strokePath(p, juce::PathStrokeType(1.5f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));

    // Draw Ratio Text
    g.setColour(juce::Colours::white.withAlpha(0.7f));
    g.setFont(14.0f);
    juce::String ratioText = juce::String::formatted("x%.2f", std::max(0.0f, lastRatio));
    g.drawText(ratioText, getLocalBounds().reduced(5), juce::Justification::bottomRight, false);
}

void OscVisualizer::mouseDown(const juce::MouseEvent& e) {
    isDragging = true;
    startWave = apvts.getParameter(idWave)->getValue();
    startRatio = apvts.getParameter(idRatio)->getValue();
}

void OscVisualizer::mouseDrag(const juce::MouseEvent& e) {
    if (!isDragging) return;
    auto bounds = getLocalBounds().toFloat().reduced(5.0f, 10.0f);
    
    // X axis for Ratio, Y axis for Wave
    float dx = e.getDistanceFromDragStartX() / 200.0f;
    float dy = -e.getDistanceFromDragStartY() / bounds.getHeight();

    float newRatio = juce::jlimit(0.0f, 1.0f, startRatio + dx);
    float newWave = juce::jlimit(0.0f, 1.0f, startWave + dy);
    
    apvts.getParameter(idRatio)->setValueNotifyingHost(newRatio);
    apvts.getParameter(idWave)->setValueNotifyingHost(newWave);
}

void OscVisualizer::mouseUp(const juce::MouseEvent& e) {
    isDragging = false;
}

// ==============================================================================
NoiseVisualizer::NoiseVisualizer(juce::AudioProcessorValueTreeState& apvts,
                                 const juce::String& paramType)
    : apvts(apvts), idType(paramType)
{
    noiseBuffer.resize(2000);
    auto& random = juce::Random::getSystemRandom();
    for (auto& v : noiseBuffer) v = random.nextFloat() * 2.0f - 1.0f;
    
    startTimerHz(30);
}

NoiseVisualizer::~NoiseVisualizer() {
    stopTimer();
}

void NoiseVisualizer::timerCallback() {
    float type = *apvts.getRawParameterValue(idType);
    if (type != lastType) {
        lastType = type;
    }
    
    // Scroll the noise
    phaseOffset -= 2.0f;
    if (phaseOffset < 0.0f) {
        phaseOffset += noiseBuffer.size();
    }
    
    repaint();
}

void NoiseVisualizer::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    
    g.setColour(juce::Colour(0xff060606));
    g.fillRoundedRectangle(bounds, 5.0f);
    
    juce::ColourGradient innerShadow(juce::Colours::black.withAlpha(0.8f), 0, bounds.getY(),
                                     juce::Colours::transparentBlack, 0, bounds.getY() + 15.0f, false);
    g.setGradientFill(innerShadow);
    g.fillRoundedRectangle(bounds, 5.0f);
    
    g.setColour(juce::Colours::black.withAlpha(0.6f));
    g.drawRoundedRectangle(bounds.reduced(1.0f), 5.0f, 1.0f);

    bounds.reduce(5.0f, 10.0f);

    float w = bounds.getWidth();
    float h = bounds.getHeight();
    float cy = bounds.getCentreY();
    float left = bounds.getX();
    
    float t = std::max(0.0f, lastType);
    juce::Colour c;
    if (t > 0.5f) {
        float mix = (t - 0.5f) * 2.0f;
        c = juce::Colours::pink.interpolatedWith(juce::Colours::white, mix);
    } else {
        float mix = t * 2.0f;
        c = juce::Colour(0xff8b4513).interpolatedWith(juce::Colours::pink, mix); // SaddleBrown
    }
    c = c.withMultipliedSaturation(0.3f);

    juce::Path p;
    bool first = true;
    float z = 0.0f;
    float alpha = 0.05f + 0.95f * t; // simple 1-pole lowpass coeff
    
    int startIdx = (int)phaseOffset;
    
    // Pre-warm the filter so it doesn't snap to 0 at the left edge
    for (int i = -100; i < 0; i += 2) {
        int idx = (startIdx + i + (int)noiseBuffer.size()) % noiseBuffer.size();
        float randVal = noiseBuffer[idx];
        z = z + alpha * (randVal - z);
    }
    
    for (float x = 0; x <= w; x += 2.0f) {
        int idx = (startIdx + (int)x) % noiseBuffer.size();
        float randVal = noiseBuffer[idx];
        z = z + alpha * (randVal - z);
        
        // boost amplitude slightly for heavier filtered signals to keep it visible
        float displayZ = z * (1.0f + (1.0f - alpha) * 2.0f); 
        displayZ = juce::jlimit(-1.0f, 1.0f, displayZ);
        
        float py = cy - displayZ * (h * 0.4f);
        if (first) {
            p.startNewSubPath(left + x, py);
            first = false;
        } else {
            p.lineTo(left + x, py);
        }
    }

    juce::Path strokedPath;
    juce::PathStrokeType(2.0f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded).createStrokedPath(strokedPath, p);
    juce::DropShadow glow(c.withAlpha(0.8f), 12, juce::Point<int>(0, 0));
    glow.drawForPath(g, strokedPath);

    g.setColour(c);
    g.strokePath(p, juce::PathStrokeType(1.5f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));
    
    // Draw Type Text
    g.setColour(juce::Colours::white.withAlpha(0.7f));
    g.setFont(14.0f);
    juce::String typeStr = "Brown";
    if (t > 0.33f && t < 0.66f) typeStr = "Pink";
    else if (t >= 0.66f) typeStr = "White";
    g.drawText(typeStr, getLocalBounds().reduced(5), juce::Justification::bottomRight, false);
}

void NoiseVisualizer::mouseDown(const juce::MouseEvent& e) {
    isDragging = true;
    startType = apvts.getParameter(idType)->getValue();
}

void NoiseVisualizer::mouseDrag(const juce::MouseEvent& e) {
    if (!isDragging) return;
    auto bounds = getLocalBounds().toFloat().reduced(5.0f, 10.0f);
    
    // Y axis for Noise Type
    float dy = -e.getDistanceFromDragStartY() / bounds.getHeight();
    float newType = juce::jlimit(0.0f, 1.0f, startType + dy);
    
    apvts.getParameter(idType)->setValueNotifyingHost(newType);
}

void NoiseVisualizer::mouseUp(const juce::MouseEvent& e) {
    isDragging = false;
}

// ==============================================================================
FilterVisualizer::FilterVisualizer(juce::AudioProcessorValueTreeState& apvts,
                                   const juce::String& paramType,
                                   const juce::String& paramRes,
                                   const juce::String& paramCutoff)
    : apvts(apvts), idType(paramType), idRes(paramRes), idCutoff(paramCutoff)
{
    startTimerHz(30);
}

FilterVisualizer::~FilterVisualizer() {
    stopTimer();
}

void FilterVisualizer::timerCallback() {
    float type = *apvts.getRawParameterValue(idType);
    float res = *apvts.getRawParameterValue(idRes);
    float cutoff = *apvts.getRawParameterValue(idCutoff);
    
    if (type != lastType || res != lastRes || cutoff != lastCutoff) {
        lastType = type;
        lastRes = res;
        lastCutoff = cutoff;
        repaint();
    }
}

void FilterVisualizer::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    
    g.setColour(juce::Colour(0xff060606));
    g.fillRoundedRectangle(bounds, 5.0f);
    
    juce::ColourGradient innerShadow(juce::Colours::black.withAlpha(0.8f), 0, bounds.getY(),
                                     juce::Colours::transparentBlack, 0, bounds.getY() + 15.0f, false);
    g.setGradientFill(innerShadow);
    g.fillRoundedRectangle(bounds, 5.0f);
    
    g.setColour(juce::Colours::black.withAlpha(0.6f));
    g.drawRoundedRectangle(bounds.reduced(1.0f), 5.0f, 1.0f);

    bounds.reduce(5.0f, 10.0f);
    
    float w = bounds.getWidth();
    float h = bounds.getHeight();
    float cy = bounds.getCentreY();
    float left = bounds.getX();
    
    float t = std::max(0.0f, lastType);
    float r = std::max(0.0f, lastRes);
    float c = std::max(0.0f, lastCutoff);
    
    int filterType = 0; // LP
    if (t > 0.33f && t < 0.66f) filterType = 1; // BP
    else if (t >= 0.66f) filterType = 2; // HP
    
    juce::Colour color = (filterType == 0) ? juce::Colours::cyan 
                       : (filterType == 1) ? juce::Colours::yellow 
                       : juce::Colours::magenta;
                       
    color = color.withMultipliedSaturation(0.3f);
                       
    juce::Path p;
    bool first = true;
    
    // Calculate Q from resonance (0 to 1 -> 0.707 to 10.0)
    float q = juce::jmap(r, 0.0f, 1.0f, 0.707f, 10.0f);
    
    for (float x = 0; x <= w; x += 1.0f) {
        float u = x / w;
        // Map u to a slightly wider range so the peak at c=0 or c=1 isn't flush against the walls.
        // We add 15% padding on the right so the high frequencies drop off cleanly at max cutoff.
        float mappedU = juce::jmap(u, 0.0f, 1.0f, -0.05f, 1.15f);
        
        // f is the frequency ratio (freq / cutoff). We use a log scale spanning 3 decades (1000x).
        float f = std::pow(1000.0f, mappedU - c);
        
        float f2 = f * f;
        float f4 = f2 * f2;
        float q2 = q * q;
        
        // Denominator of the SVF magnitude response squared
        float num2 = 1.0f;
        if (filterType == 0) { // LP
            num2 = 1.0f;
        } else if (filterType == 1) { // BP
            num2 = f2 / q2;
        } else { // HP
            num2 = f4;
        }
        
        float den2 = (1.0f - f2) * (1.0f - f2) + (f2 / q2);
        
        float mag2 = num2 / den2;
        float db = 10.0f * std::log10(std::max(mag2, 1e-12f)); // clamp to -120dB min
        
        // Map dB to Y pixel: 0dB is slightly below center, +20dB is near top, -40dB is bottom
        // Scale: 20dB = h * 0.4f
        float py = (cy + h * 0.1f) - (db / 20.0f) * (h * 0.4f);
        
        // Clamp visually just outside the bounds to avoid drawing a flat line inside the box
        py = juce::jlimit(-bounds.getHeight(), bounds.getBottom() + 20.0f, py);
        
        if (first) {
            p.startNewSubPath(left + x, py);
            first = false;
        } else {
            p.lineTo(left + x, py);
        }
    }

    // Gradient fill under the curve
    juce::Path filledPath = p;
    filledPath.lineTo(left + w, bounds.getBottom());
    filledPath.lineTo(left, bounds.getBottom());
    filledPath.closeSubPath();
    
    juce::ColourGradient grad(color.withAlpha(0.2f), 0, cy - h * 0.3f, color.withAlpha(0.0f), 0, bounds.getBottom(), false);
    g.setGradientFill(grad);
    g.fillPath(filledPath);

    juce::Path strokedPath;
    juce::PathStrokeType(2.0f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded).createStrokedPath(strokedPath, p);
    juce::DropShadow glow(color.withAlpha(0.8f), 12, juce::Point<int>(0, 0));
    glow.drawForPath(g, strokedPath);

    g.setColour(color);
    g.strokePath(p, juce::PathStrokeType(1.5f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));
    
    // Draw Text
    g.setColour(juce::Colours::white.withAlpha(0.7f));
    g.setFont(14.0f);
    juce::String typeStr = (filterType == 0) ? "Lowpass" : (filterType == 1) ? "Bandpass" : "Highpass";
    g.drawText(typeStr, getLocalBounds().reduced(5), juce::Justification::bottomRight, false);
}

void FilterVisualizer::mouseDown(const juce::MouseEvent& e) {
    isDragging = true;
    startType = apvts.getParameter(idType)->getValue();
    startRes = apvts.getParameter(idRes)->getValue();
}

void FilterVisualizer::mouseDrag(const juce::MouseEvent& e) {
    if (!isDragging) return;
    auto bounds = getLocalBounds().toFloat().reduced(5.0f, 10.0f);
    
    float dx = e.getDistanceFromDragStartX() / bounds.getWidth();
    float dy = -e.getDistanceFromDragStartY() / bounds.getHeight();
    
    float newType = juce::jlimit(0.0f, 1.0f, startType + dx);
    float newRes = juce::jlimit(0.0f, 1.0f, startRes + dy);
    
    apvts.getParameter(idType)->setValueNotifyingHost(newType);
    apvts.getParameter(idRes)->setValueNotifyingHost(newRes);
}

void FilterVisualizer::mouseUp(const juce::MouseEvent& e) {
    isDragging = false;
}

} // namespace genesynth
