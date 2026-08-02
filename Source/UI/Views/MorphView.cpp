#include "MorphView.h"
#include "../../Analysis/PitchTracker.h"
#include "../../Analysis/AudioResampler.h"
#include "../../Analysis/FeatureExtractor.h"
#include "../../Analysis/InferenceEngine.h"

namespace genesynth {

// ==============================================================================
MorphPadComponent::MorphPadComponent()
{
    // Initialize some random nodes for the placeholder latent space
    juce::Random r;
    for (int i = 0; i < 12; ++i) {
        float hue = r.nextFloat();
        juce::Colour c = juce::Colour::fromHSV(hue, 0.08f, 1.0f, 1.0f);
        nodes.push_back({ juce::Point<float>(r.nextFloat(), r.nextFloat()), false, c });
    }
    startTimerHz(60);
}

MorphPadComponent::~MorphPadComponent()
{
    stopTimer();
}

void MorphPadComponent::timerCallback()
{
    animPhase += 1.0f;
    repaint();
}

void MorphPadComponent::paint(juce::Graphics& g)
{
    auto outerBounds = getLocalBounds().toFloat();
    auto b = outerBounds.reduced(14.0f); // Make room for the 4 shadows
    
    // 1. Draw outer shadows (the bulge)
    if (outerShadowImage.isValid()) {
        g.drawImageAt(outerShadowImage, 0, 0);
    }
    
    // Background (darker screen inset)
    g.setColour(juce::Colour(0xff060606));
    g.fillRoundedRectangle(b, 10.0f);
    
    // Clip everything that follows (inner shadow, grid, nodes) to the pad bounds
    juce::Path clipPath;
    clipPath.addRoundedRectangle(b, 10.0f);
    g.reduceClipRegion(clipPath);
    
    // Draw inner shadow
    if (innerShadowImage.isValid()) {
        g.drawImageAt(innerShadowImage, 0, 0);
    }
    
    // Draw simulated 3D dot grid
    {
        float t = animPhase * 0.001f; // Extremely slow time for subtle, ambient rotation
        float angleY = t * 0.2f;
        float angleX = std::sin(t * 0.15f) * 0.2f;
        float cosY = std::cos(angleY), sinY = std::sin(angleY);
        float cosX = std::cos(angleX), sinX = std::sin(angleX);
        
        int gridSize = 14; // Larger grid size for a denser cloud
        float spacing = 5.0f; // Adjusted spacing
        float offset = (gridSize - 1) * spacing * 0.5f;
        float radiusSq = offset * offset; // Sphere radius squared
        float cx = b.getCentreX();
        float cy = b.getCentreY();
        
        for (int x = 0; x < gridSize; ++x) {
            for (int y = 0; y < gridSize; ++y) {
                for (int z = 0; z < gridSize; ++z) {
                    float px = x * spacing - offset;
                    float py = y * spacing - offset;
                    float pz = z * spacing - offset;
                    
                    // Carve a sphere out of the cube
                    if (px*px + py*py + pz*pz > radiusSq) continue;
                    
                    float px1 = px * cosY - pz * sinY;
                    float pz1 = px * sinY + pz * cosY;
                    
                    float py2 = py * cosX - pz1 * sinX;
                    float pz2 = py * sinX + pz1 * cosX;
                    
                    float z_final = pz2 + 15.0f; // Camera is inside the huge cube
                    
                    if (z_final > 1.0f) { // Draw points that are in front of the camera
                        float projScale = 800.0f / z_final;
                        float sx = cx + px1 * projScale;
                        float sy = cy + py2 * projScale;
                        
                        // Fade out smoothly when getting too close to avoid high-speed "jumps"
                        float alphaNear = juce::jlimit(0.0f, 1.0f, (z_final - 3.0f) * 0.5f); 
                        float alphaFar = juce::jlimit(0.0f, 1.0f, 25.0f / z_final);
                        float alpha = alphaNear * alphaFar * 0.7f;
                        
                        float size = juce::jlimit(0.5f, 6.0f, 40.0f / z_final);
                        
                        g.setColour(juce::Colours::white.withAlpha(alpha));
                        g.fillRect(sx - size * 0.5f, sy - size * 0.5f, size, size); // fillRect is faster than fillEllipse
                    }
                }
            }
        }
    }
    
    
    g.setColour(juce::Colours::white);
    g.setFont(20.0f);
    g.drawText("morph", b.withTrimmedLeft(15.0f).withTrimmedTop(15.0f), juce::Justification::topLeft);
    
    // Compute connections: every node connects to its 2 nearest neighbors + any within 200px
    std::vector<std::pair<int, int>> connections;
    for (int i = 0; i < (int)nodes.size(); ++i) {
        auto p1 = juce::Point<float>(b.getX() + nodes[i].pos.x * b.getWidth(), b.getY() + nodes[i].pos.y * b.getHeight());
        int closest1 = -1, closest2 = -1;
        float d1 = 1e9f, d2 = 1e9f;
        for (int j = 0; j < (int)nodes.size(); ++j) {
            if (i == j) continue;
            auto p2 = juce::Point<float>(b.getX() + nodes[j].pos.x * b.getWidth(), b.getY() + nodes[j].pos.y * b.getHeight());
            float d = p1.getDistanceFrom(p2);
            if (d < d1) { d2 = d1; closest2 = closest1; d1 = d; closest1 = j; }
            else if (d < d2) { d2 = d; closest2 = j; }
        }
        
        auto addConn = [&](int target) {
            if (target != -1) {
                int a = std::min(i, target); int c = std::max(i, target);
                if (std::find(connections.begin(), connections.end(), std::make_pair(a, c)) == connections.end())
                    connections.push_back({a, c});
            }
        };
        addConn(closest1);
        addConn(closest2);
        
        // Add distance-based local connections
        for (int j = i + 1; j < (int)nodes.size(); ++j) {
            auto p2 = juce::Point<float>(b.getX() + nodes[j].pos.x * b.getWidth(), b.getY() + nodes[j].pos.y * b.getHeight());
            if (p1.getDistanceFrom(p2) < 200.0f) {
                addConn(j);
            }
        }
    }
    
    float tNodes = animPhase * 0.05f; // time driver for nodes
    
    auto getNodePulse = [&](int idx) {
        return (std::sin(tNodes + (float)idx * 0.5f) + 1.0f) * 0.5f; // 0.0 to 1.0
    };
    
    // Draw Connections
    for (auto& conn : connections) {
        auto p1 = juce::Point<float>(b.getX() + nodes[conn.first].pos.x * b.getWidth(), b.getY() + nodes[conn.first].pos.y * b.getHeight());
        auto p2 = juce::Point<float>(b.getX() + nodes[conn.second].pos.x * b.getWidth(), b.getY() + nodes[conn.second].pos.y * b.getHeight());
        
        juce::Colour c1 = nodes[conn.first].color;
        juce::Colour c2 = nodes[conn.second].color;
        
        float pulse1 = getNodePulse(conn.first);
        float pulse2 = getNodePulse(conn.second);
        
        float t1 = 0.5f + 1.5f * pulse1; // Core half-thickness 0.5 to 2.0
        float t2 = 0.5f + 1.5f * pulse2;
        
        float gt1 = 2.0f + 3.0f * pulse1; // Glow half-thickness 2.0 to 5.0
        float gt2 = 2.0f + 3.0f * pulse2;
        
        juce::Point<float> dir = p2 - p1;
        float dist = dir.getDistanceFromOrigin();
        if (dist > 0.001f) {
            dir /= dist;
            juce::Point<float> norm(-dir.y, dir.x);
            
            juce::Path corePath;
            corePath.startNewSubPath(p1 + norm * t1);
            corePath.lineTo(p1 - norm * t1);
            corePath.lineTo(p2 - norm * t2);
            corePath.lineTo(p2 + norm * t2);
            corePath.closeSubPath();
            
            juce::Path glowPath;
            glowPath.startNewSubPath(p1 + norm * gt1);
            glowPath.lineTo(p1 - norm * gt1);
            glowPath.lineTo(p2 - norm * gt2);
            glowPath.lineTo(p2 + norm * gt2);
            glowPath.closeSubPath();
            
            juce::ColourGradient glowGrad(c1.withAlpha(0.15f), p1, c2.withAlpha(0.15f), p2, false);
            g.setGradientFill(glowGrad);
            g.fillPath(glowPath);
            
            juce::ColourGradient coreGrad(c1.withAlpha(0.6f), p1, c2.withAlpha(0.6f), p2, false);
            g.setGradientFill(coreGrad);
            g.fillPath(corePath);
        }
    }

    // Draw Nodes
    int i = 0;
    for (auto& n : nodes) {
        auto px = b.getX() + n.pos.x * b.getWidth();
        auto py = b.getY() + n.pos.y * b.getHeight();
        
        float nodePulse = getNodePulse(i);
        float glowRadius = 10.0f + 4.0f * nodePulse; // 10 to 14
        float coreRadius = 4.0f + 2.0f * nodePulse;  // 4 to 6
        
        // --- Ambient Humidity Halo (YouTube style diffuse glow) ---
        float ambientRadius = 150.0f + 50.0f * nodePulse;
        juce::Colour ambientColor = n.color.withMultipliedSaturation(3.0f).withAlpha(0.04f + 0.02f * nodePulse);
        juce::ColourGradient ambientGrad(ambientColor, px, py,
                                         juce::Colours::transparentBlack, px + ambientRadius, py, true);
        g.setGradientFill(ambientGrad);
        g.fillEllipse(px - ambientRadius, py - ambientRadius, ambientRadius * 2.0f, ambientRadius * 2.0f);
        
        // Glow
        g.setColour(n.color.withAlpha(0.15f + 0.1f * nodePulse));
        g.fillEllipse(px - glowRadius, py - glowRadius, glowRadius * 2.0f, glowRadius * 2.0f);
        
        if (n.isDragging) {
            g.setColour(juce::Colours::white);
            g.fillEllipse(px - 8.0f, py - 8.0f, 16.0f, 16.0f);
        } else {
            g.setColour(n.color.withAlpha(0.8f));
            g.fillEllipse(px - coreRadius, py - coreRadius, coreRadius * 2.0f, coreRadius * 2.0f);
            g.setColour(juce::Colours::white);
            g.drawEllipse(px - coreRadius, py - coreRadius, coreRadius * 2.0f, coreRadius * 2.0f, 2.0f);
        }
        i++;
    }
}

void MorphPadComponent::resized()
{
    auto outerBounds = getLocalBounds().toFloat();
    auto b = outerBounds.reduced(14.0f);
    if (b.getWidth() > 0 && b.getHeight() > 0) {
        // 1. Generate OUTER shadows (The Bulge)
        outerShadowImage = juce::Image(juce::Image::ARGB, getWidth(), getHeight(), true);
        juce::Graphics og(outerShadowImage);
        
        juce::Path solidPath;
        solidPath.addRoundedRectangle(b, 10.0f);
        
        // Outer Top-Left Light (Bulge highlight)
        juce::DropShadow outerLight(juce::Colours::white.withAlpha(0.06f), 15, { -4, -4 });
        outerLight.drawForPath(og, solidPath);
        
        // Outer Bottom-Right Dark (Bulge shadow)
        juce::DropShadow outerDark(juce::Colours::black.withAlpha(0.5f), 15, { 5, 5 });
        outerDark.drawForPath(og, solidPath);

        // 2. Generate INNER shadows (The Recess)
        innerShadowImage = juce::Image(juce::Image::ARGB, getWidth(), getHeight(), true);
        juce::Graphics ig(innerShadowImage);
        
        juce::Path outlinePath;
        outlinePath.addRoundedRectangle(b.expanded(6.0f), 10.0f);
        juce::PathStrokeType stroke(12.0f);
        stroke.createStrokedPath(outlinePath, outlinePath);
        
        // Inner Top-Left Dark (Recess shadow)
        juce::DropShadow innerDark(juce::Colours::black.withAlpha(0.6f), 15, { 5, 5 });
        innerDark.drawForPath(ig, outlinePath);
        
        // Inner Bottom-Right Light (Recess highlight)
        juce::DropShadow innerLight(juce::Colours::white.withAlpha(0.08f), 15, { -5, -5 });
        innerLight.drawForPath(ig, outlinePath);
    }
}

void MorphPadComponent::mouseDown(const juce::MouseEvent& e)
{
    auto outerBounds = getLocalBounds().toFloat();
    auto b = outerBounds.reduced(14.0f);
    auto mousePos = e.position;
    
    draggedNodeIndex = -1;
    float minDistance = 20.0f; // Click radius
    
    for (size_t i = 0; i < nodes.size(); ++i) {
        auto px = b.getX() + nodes[i].pos.x * b.getWidth();
        auto py = b.getY() + nodes[i].pos.y * b.getHeight();
        juce::Point<float> nodePos(px, py);
        
        if (mousePos.getDistanceFrom(nodePos) < minDistance) {
            draggedNodeIndex = (int)i;
            nodes[i].isDragging = true;
            break;
        }
    }
    
    if (draggedNodeIndex != -1)
        repaint();
}

void MorphPadComponent::mouseDrag(const juce::MouseEvent& e)
{
    if (draggedNodeIndex >= 0 && draggedNodeIndex < (int)nodes.size()) {
        auto outerBounds = getLocalBounds().toFloat();
        auto b = outerBounds.reduced(14.0f);
        auto& n = nodes[draggedNodeIndex];
        n.pos.x = juce::jlimit(0.0f, 1.0f, (e.position.x - b.getX()) / b.getWidth());
        n.pos.y = juce::jlimit(0.0f, 1.0f, (e.position.y - b.getY()) / b.getHeight());
        
        if (onNodeMoved)
            onNodeMoved(n.pos.x, n.pos.y);
            
        repaint();
    }
}

void MorphPadComponent::mouseUp(const juce::MouseEvent&)
{
    if (draggedNodeIndex != -1) {
        nodes[draggedNodeIndex].isDragging = false;
        draggedNodeIndex = -1;
        repaint();
    }
}


// ==============================================================================
MorphBottomBar::MorphBottomBar(juce::AudioProcessorValueTreeState& apvts, genesynth::InferenceEngine* inferenceEngine)
    : thumbnail(512, formatManager, thumbnailCache), inferenceEngine(inferenceEngine)
{
    formatManager.registerBasicFormats();
    thumbnail.addChangeListener(this);
    
    addAndMakeVisible(importBtn);
    addAndMakeVisible(generateBtn);
    
    importBtn.onClick = [this] {
        fileChooser = std::make_unique<juce::FileChooser>("Select a .wav file...",
                                                          juce::File::getSpecialLocation(juce::File::userMusicDirectory),
                                                          "*.wav");
        
        auto folderChooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
        
        fileChooser->launchAsync(folderChooserFlags, [this] (const juce::FileChooser& fc) {
            auto file = fc.getResult();
            if (file != juce::File{}) {
                loadFile(file);
            }
        });
    };
    
    generateBtn.onClick = [this] {
        if (this->inferenceEngine != nullptr) {
            juce::Logger::writeToLog("Generate Button clicked: Triggering fake inference...");
            // Fake feature vector: 86 frames * 128 mels (filled with random data or just 0.5)
            std::vector<float> fakeFeatures(86 * 128, 0.5f);
            this->inferenceEngine->requestInference(fakeFeatures);
        }
    };
    
    auto addKnob = [&](LabeledSlider& k, const juce::String& paramId, const juce::String& name) {
        k.slider = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox);
        k.label = std::make_unique<juce::Label>("", name);
        k.label->setJustificationType(juce::Justification::centred);
        k.label->setFont(12.0f);
        addAndMakeVisible(*k.slider);
        addAndMakeVisible(*k.label);
        k.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, paramId, *k.slider);
    };

    addKnob(spiceKnob, "morph_spice", "spice");
    addKnob(brightKnob, "morph_bright", "bright");
    addKnob(timeKnob, "morph_time", "time");
}

void MorphBottomBar::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &thumbnail) {
        repaint();
    }
}

bool MorphBottomBar::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (auto file : files) {
        if (file.endsWithIgnoreCase(".wav"))
            return true;
    }
    return false;
}

void MorphBottomBar::filesDropped(const juce::StringArray& files, int x, int y)
{
    for (auto file : files) {
        if (file.endsWithIgnoreCase(".wav")) {
            loadFile(juce::File(file));
            break;
        }
    }
}

void MorphBottomBar::loadFile(const juce::File& file)
{
    thumbnail.setSource(new juce::FileInputSource(file));

    juce::Thread::launch([this, file]() {
        std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
        if (reader == nullptr) {
            juce::Logger::writeToLog("Failed to read audio file: " + file.getFullPathName());
            return;
        }

        juce::AudioBuffer<float> buffer(1, (int)reader->lengthInSamples);
        reader->read(&buffer, 0, (int)reader->lengthInSamples, 0, true, false);

        double originalSampleRate = reader->sampleRate;

        // 1. Pitch Tracking
        float fundFreq = genesynth::PitchTracker::detectPitchMPM(buffer.getReadPointer(0), buffer.getNumSamples(), originalSampleRate);
        juce::Logger::writeToLog("Pitch detected: " + juce::String(fundFreq) + " Hz");

        if (fundFreq > 0.0f) {
            // 2. Repitching
            auto repitchedBuffer = genesynth::AudioResampler::repitchToC5(buffer, fundFreq);

            // 3. Resample to Target Rate (22050 Hz)
            auto finalBuffer = genesynth::AudioResampler::resampleToTargetRate(repitchedBuffer, originalSampleRate, 22050.0);

            // 4. Feature Extraction
            genesynth::FeatureExtractor extractor;
            auto features = extractor.extractFeatures(finalBuffer, 22050.0);
            juce::Logger::writeToLog("Extracted " + juce::String(features.numFrames) + " frames of Log-Mel Spectrogram.");

            // 5. Inference
            if (inferenceEngine != nullptr && features.numFrames > 0) {
                juce::Logger::writeToLog("Triggering Inference...");
                inferenceEngine->requestInference(features.logMelSpectrogram);
            }
        }
    });
}

void MorphBottomBar::paint(juce::Graphics& g)
{
    auto b = getLocalBounds();
    auto sampleArea = b.withTrimmedLeft(60).withWidth(120).reduced(0, 5);
    
    g.setColour(juce::Colours::black);
    g.fillRoundedRectangle(sampleArea.toFloat(), 5.0f);
    
    if (thumbnail.getNumChannels() == 0) {
        g.setColour(juce::Colours::white);
        g.drawRoundedRectangle(sampleArea.toFloat(), 5.0f, 2.0f);
        g.setFont(12.0f);
        g.drawText("sample", sampleArea.withY(sampleArea.getY() + 5).withHeight(15), juce::Justification::centredTop);
    } else {
        g.setColour(juce::Colours::white.withAlpha(0.7f));
        thumbnail.drawChannel(g, sampleArea.reduced(2), 0.0, thumbnail.getTotalLength(), 0, 1.0f);
        
        g.setColour(juce::Colours::white);
        g.drawRoundedRectangle(sampleArea.toFloat(), 5.0f, 2.0f);
    }
}

void MorphBottomBar::resized()
{
    auto b = getLocalBounds();
    
    importBtn.setBounds(b.removeFromLeft(50).reduced(0, 10));
    b.removeFromLeft(130); // sample display area
    
    b.removeFromLeft(20);
    
    auto knobArea = b.removeFromLeft(60);
    spiceKnob.slider->setBounds(knobArea.reduced(5, 5).withTrimmedBottom(15));
    spiceKnob.label->setBounds(knobArea.withTop(knobArea.getBottom() - 15));
    
    knobArea = b.removeFromLeft(60);
    brightKnob.slider->setBounds(knobArea.reduced(5, 5).withTrimmedBottom(15));
    brightKnob.label->setBounds(knobArea.withTop(knobArea.getBottom() - 15));
    
    knobArea = b.removeFromLeft(60);
    timeKnob.slider->setBounds(knobArea.reduced(5, 5).withTrimmedBottom(15));
    timeKnob.label->setBounds(knobArea.withTop(knobArea.getBottom() - 15));
    
    b.removeFromLeft(20);
    generateBtn.setBounds(b.removeFromLeft(80).reduced(0, 10));
}


// ==============================================================================
MorphView::MorphView(juce::AudioProcessorValueTreeState& apvts, genesynth::InferenceEngine* inferenceEngine)
    : bottomBar(apvts, inferenceEngine)
{
    addAndMakeVisible(pad);
    addAndMakeVisible(bottomBar);
    
    pad.onNodeMoved = [inferenceEngine](float x, float y) {
        if (inferenceEngine != nullptr) {
            // We use the X and Y coordinates to fill the fake features.
            // This will cause the linear layer of the dummy model to output
            // smoothly interpolated parameters as we drag!
            float featureValue = (x + y) * 0.5f; 
            std::vector<float> fakeFeatures(86 * 128, featureValue);
            inferenceEngine->requestInference(fakeFeatures);
        }
    };
}

void MorphView::resized()
{
    auto b = getLocalBounds();
    b.reduce(14, 0); // 10px from PluginEditor + 14px = 24px side margin to match SynthView
    b.removeFromBottom(14); // Match bottom padding

    bottomBar.setBounds(b.removeFromBottom(60));
    b.removeFromBottom(10); // gap
    pad.setBounds(b);
}

} // namespace genesynth
