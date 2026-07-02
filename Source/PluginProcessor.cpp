#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Parameters/StaticParameter.h"
#include "Parameters/ModulableParameter.h"
#include "Parameters/ParameterFactory.h"
#include "Framework/aBlock/aContainer/SynthEngine/PolyphonyManager/PolyphonyManager.h"
#include "Framework/aBlock/aContainer/SynthEngine/PolyphonyManager/SynthVoice/SynthVoice.h"
GeneSynthAudioProcessor::GeneSynthAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), apvts(*this, nullptr, "Parameters", createParameterLayout())
#endif
{
    using namespace genesynth;
    
        // 1. Instanciation des paramètres dans allParams
    // Générateurs
    allParams.push_back(std::make_unique<StaticParameter>(ParamID::OSC_A_WAVE, "osc_a_wave", 0.0f));
    allParams.push_back(std::make_unique<StaticParameter>(ParamID::OSC_A_RATIO, "osc_a_ratio", 1.0f));
    allParams.push_back(std::make_unique<ModulableParameter>(ParamID::OSC_A_VOL_BASE, "osc_a_vol_base", 0.5f));
    allParams.push_back(std::make_unique<StaticParameter>(ParamID::OSC_A_VOL_MOD, "osc_a_vol_mod", 0.5f));
    allParams.push_back(std::make_unique<ModulableParameter>(ParamID::OSC_A_FREQ_BASE, "osc_a_freq_base", 0.0f));
    allParams.push_back(std::make_unique<StaticParameter>(ParamID::OSC_A_FREQ_MOD, "osc_a_freq_mod", 0.5f));
    allParams.push_back(std::make_unique<ModulableParameter>(ParamID::OSC_A_FM_BASE, "osc_a_fm_base", 0.0f));
    allParams.push_back(std::make_unique<StaticParameter>(ParamID::OSC_A_FM_MOD, "osc_a_fm_mod", 0.5f));

    allParams.push_back(std::make_unique<StaticParameter>(ParamID::OSC_B_WAVE, "osc_b_wave", 0.0f));
    allParams.push_back(std::make_unique<StaticParameter>(ParamID::OSC_B_RATIO, "osc_b_ratio", 1.0f));
    allParams.push_back(std::make_unique<ModulableParameter>(ParamID::OSC_B_VOL_BASE, "osc_b_vol_base", 0.5f));
    allParams.push_back(std::make_unique<StaticParameter>(ParamID::OSC_B_VOL_MOD, "osc_b_vol_mod", 0.5f));
    allParams.push_back(std::make_unique<ModulableParameter>(ParamID::OSC_B_FREQ_BASE, "osc_b_freq_base", 0.0f));
    allParams.push_back(std::make_unique<StaticParameter>(ParamID::OSC_B_FREQ_MOD, "osc_b_freq_mod", 0.5f));

    allParams.push_back(std::make_unique<StaticParameter>(ParamID::NOISE_TYPE, "noise_type", 0.5f));
    allParams.push_back(std::make_unique<ModulableParameter>(ParamID::NOISE_VOL_BASE, "noise_vol_base", 0.0f));
    allParams.push_back(std::make_unique<StaticParameter>(ParamID::NOISE_VOL_MOD, "noise_vol_mod", 0.5f));
    allParams.push_back(std::make_unique<ModulableParameter>(ParamID::NOISE_FREQ_BASE, "noise_freq_base", 0.5f));
    allParams.push_back(std::make_unique<StaticParameter>(ParamID::NOISE_FREQ_MOD, "noise_freq_mod", 0.5f));

    // Filtre
    allParams.push_back(std::make_unique<StaticParameter>(ParamID::FILTER_TYPE, "filter_type", 0.0f));
    allParams.push_back(std::make_unique<StaticParameter>(ParamID::FILTER_RES, "filter_res", 0.0f));
    allParams.push_back(std::make_unique<ModulableParameter>(ParamID::FILTER_CUTOFF_BASE, "filter_cutoff_base", 1.0f));
    allParams.push_back(std::make_unique<StaticParameter>(ParamID::FILTER_CUTOFF_MOD, "filter_cutoff_mod", 0.5f));
    allParams.push_back(std::make_unique<ModulableParameter>(ParamID::FILTER_MIX_BASE, "filter_mix_base", 1.0f));
    allParams.push_back(std::make_unique<StaticParameter>(ParamID::FILTER_MIX_MOD, "filter_mix_mod", 0.5f));

    // Modulation Sources
    allParams.push_back(std::make_unique<StaticParameter>(ParamID::ENV_MOD_A, "env_mod_a", 0.1f));
    allParams.push_back(std::make_unique<StaticParameter>(ParamID::ENV_MOD_D, "env_mod_d", 0.1f));
    allParams.push_back(std::make_unique<StaticParameter>(ParamID::ENV_MOD_S, "env_mod_s", 1.0f));
    allParams.push_back(std::make_unique<StaticParameter>(ParamID::ENV_MOD_R, "env_mod_r", 0.1f));
    allParams.push_back(std::make_unique<StaticParameter>(ParamID::LFO_AMT, "lfo_amt", 1.0f));
    allParams.push_back(std::make_unique<StaticParameter>(ParamID::LFO_FREQ, "lfo_freq", 0.5f));

    // Enveloppe Globale
    allParams.push_back(std::make_unique<StaticParameter>(ParamID::ENV_GLOBAL_A, "env_global_a", 0.1f));
    allParams.push_back(std::make_unique<StaticParameter>(ParamID::ENV_GLOBAL_D, "env_global_d", 0.1f));
    allParams.push_back(std::make_unique<StaticParameter>(ParamID::ENV_GLOBAL_S, "env_global_s", 1.0f));
    allParams.push_back(std::make_unique<StaticParameter>(ParamID::ENV_GLOBAL_R, "env_global_r", 0.1f));

    // Effets
    allParams.push_back(std::make_unique<StaticParameter>(ParamID::CLIPPER_VOL, "clipper_vol", 0.5f));
    allParams.push_back(std::make_unique<StaticParameter>(ParamID::CLIPPER_THRESH, "clipper_thresh", 1.0f));
    allParams.push_back(std::make_unique<StaticParameter>(ParamID::REVERB_DECAY, "reverb_decay", 0.5f));
    allParams.push_back(std::make_unique<StaticParameter>(ParamID::REVERB_SIZE, "reverb_size", 0.5f));
    allParams.push_back(std::make_unique<StaticParameter>(ParamID::REVERB_MIX, "reverb_mix", 0.0f));

    // System
    allParams.push_back(std::make_unique<StaticParameter>(ParamID::GATE, "gate", 0.0f));

    // Lier les attenuateurs bipolaires aux parametres modulables
    getParamAs<ModulableParameter>(ParamID::OSC_A_VOL_BASE)->setModAmountParam(getParamAs<StaticParameter>(ParamID::OSC_A_VOL_MOD));
    getParamAs<ModulableParameter>(ParamID::OSC_A_FREQ_BASE)->setModAmountParam(getParamAs<StaticParameter>(ParamID::OSC_A_FREQ_MOD));
    getParamAs<ModulableParameter>(ParamID::OSC_A_FM_BASE)->setModAmountParam(getParamAs<StaticParameter>(ParamID::OSC_A_FM_MOD));
    
    getParamAs<ModulableParameter>(ParamID::OSC_B_VOL_BASE)->setModAmountParam(getParamAs<StaticParameter>(ParamID::OSC_B_VOL_MOD));
    getParamAs<ModulableParameter>(ParamID::OSC_B_FREQ_BASE)->setModAmountParam(getParamAs<StaticParameter>(ParamID::OSC_B_FREQ_MOD));
    
    getParamAs<ModulableParameter>(ParamID::NOISE_VOL_BASE)->setModAmountParam(getParamAs<StaticParameter>(ParamID::NOISE_VOL_MOD));
    getParamAs<ModulableParameter>(ParamID::NOISE_FREQ_BASE)->setModAmountParam(getParamAs<StaticParameter>(ParamID::NOISE_FREQ_MOD));
    
    getParamAs<ModulableParameter>(ParamID::FILTER_CUTOFF_BASE)->setModAmountParam(getParamAs<StaticParameter>(ParamID::FILTER_CUTOFF_MOD));
    getParamAs<ModulableParameter>(ParamID::FILTER_MIX_BASE)->setModAmountParam(getParamAs<StaticParameter>(ParamID::FILTER_MIX_MOD));

        // 2. Création et injection des voix
    auto& synth = engine.getPolyphonyManager().getJuceSynth();
    for (int i = 0; i < 8; ++i) {
        synth.addVoice(new SynthVoice(
            getParamAs<ModulableParameter>(ParamID::OSC_A_VOL_BASE),
            getParamAs<ModulableParameter>(ParamID::OSC_A_FREQ_BASE),
            getParamAs<StaticParameter>(ParamID::OSC_A_WAVE),
            getParamAs<StaticParameter>(ParamID::OSC_A_RATIO),
            getParamAs<ModulableParameter>(ParamID::OSC_A_FM_BASE),
            
            getParamAs<ModulableParameter>(ParamID::OSC_B_VOL_BASE),
            getParamAs<ModulableParameter>(ParamID::OSC_B_FREQ_BASE),
            getParamAs<StaticParameter>(ParamID::OSC_B_WAVE),
            getParamAs<StaticParameter>(ParamID::OSC_B_RATIO),
            
            getParamAs<ModulableParameter>(ParamID::NOISE_VOL_BASE),
            getParamAs<ModulableParameter>(ParamID::NOISE_FREQ_BASE),
            getParamAs<StaticParameter>(ParamID::NOISE_TYPE),
            
            getParamAs<StaticParameter>(ParamID::FILTER_TYPE),
            getParamAs<StaticParameter>(ParamID::FILTER_RES),
            getParamAs<ModulableParameter>(ParamID::FILTER_CUTOFF_BASE),
            getParamAs<ModulableParameter>(ParamID::FILTER_MIX_BASE),
            
            getParamAs<StaticParameter>(ParamID::CLIPPER_THRESH),
            
            getParamAs<StaticParameter>(ParamID::ENV_GLOBAL_A),
            getParamAs<StaticParameter>(ParamID::ENV_GLOBAL_D),
            getParamAs<StaticParameter>(ParamID::ENV_GLOBAL_S),
            getParamAs<StaticParameter>(ParamID::ENV_GLOBAL_R),
            getParamAs<StaticParameter>(ParamID::GATE),
            getParamAs<StaticParameter>(ParamID::ENV_MOD_A),
            getParamAs<StaticParameter>(ParamID::ENV_MOD_D),
            getParamAs<StaticParameter>(ParamID::ENV_MOD_S),
            getParamAs<StaticParameter>(ParamID::ENV_MOD_R),
            getParamAs<StaticParameter>(ParamID::LFO_AMT),
            getParamAs<StaticParameter>(ParamID::LFO_FREQ)
        ));
    }
        
    engine.getReverb().setParameters(
        getParamAs<StaticParameter>(ParamID::REVERB_DECAY),
        getParamAs<StaticParameter>(ParamID::REVERB_SIZE),
        getParamAs<StaticParameter>(ParamID::REVERB_MIX)
    );
}

GeneSynthAudioProcessor::~GeneSynthAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout GeneSynthAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Generators (Osc A)
    params.push_back(std::make_unique<juce::AudioParameterFloat>("osc_a_wave", "Osc A Wave", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("osc_a_ratio", "Osc A Ratio", 0.125f, 8.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("osc_a_vol_base", "Osc A Vol", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("osc_a_vol_mod", "Osc A Vol Mod", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("osc_a_freq_base", "Osc A Freq", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("osc_a_freq_mod", "Osc A Freq Mod", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("osc_a_fm_base", "Osc A FM", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("osc_a_fm_mod", "Osc A FM Mod", 0.0f, 1.0f, 0.5f));

    // Generators (Osc B)
    params.push_back(std::make_unique<juce::AudioParameterFloat>("osc_b_wave", "Osc B Wave", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("osc_b_ratio", "Osc B Ratio", 0.125f, 8.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("osc_b_vol_base", "Osc B Vol", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("osc_b_vol_mod", "Osc B Vol Mod", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("osc_b_freq_base", "Osc B Freq", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("osc_b_freq_mod", "Osc B Freq Mod", 0.0f, 1.0f, 0.5f));

    // Generators (Noise)
    params.push_back(std::make_unique<juce::AudioParameterFloat>("noise_type", "Noise Type", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("noise_vol_base", "Noise Vol", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("noise_vol_mod", "Noise Vol Mod", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("noise_freq_base", "Noise Freq", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("noise_freq_mod", "Noise Freq Mod", 0.0f, 1.0f, 0.5f));

    // Filter
    params.push_back(std::make_unique<juce::AudioParameterFloat>("filter_type", "Filter Type", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("filter_res", "Filter Res", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("filter_cutoff_base", "Filter Cutoff", 0.0f, 1.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("filter_cutoff_mod", "Filter Cutoff Mod", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("filter_mix_base", "Filter Mix", 0.0f, 1.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("filter_mix_mod", "Filter Mix Mod", 0.0f, 1.0f, 0.5f));

    // Modulation Sources
    params.push_back(std::make_unique<juce::AudioParameterFloat>("env_mod_a", "Mod Env A", 0.0f, 1.0f, 0.1f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("env_mod_d", "Mod Env D", 0.0f, 1.0f, 0.1f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("env_mod_s", "Mod Env S", 0.0f, 1.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("env_mod_r", "Mod Env R", 0.0f, 1.0f, 0.1f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("lfo_amt", "LFO Amt", 0.0f, 1.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("lfo_freq", "LFO Freq", 0.0f, 1.0f, 0.5f));

    // Global Envelope
    params.push_back(std::make_unique<juce::AudioParameterFloat>("env_global_a", "Global Env A", 0.0f, 1.0f, 0.1f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("env_global_d", "Global Env D", 0.0f, 1.0f, 0.1f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("env_global_s", "Global Env S", 0.0f, 1.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("env_global_r", "Global Env R", 0.0f, 1.0f, 0.1f));

    // Effects
    params.push_back(std::make_unique<juce::AudioParameterFloat>("clipper_vol", "Clipper Vol", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("clipper_thresh", "Clipper Thresh", 0.0f, 1.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("reverb_decay", "Reverb Decay", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("reverb_size", "Reverb Size", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("reverb_mix", "Reverb Mix", 0.0f, 1.0f, 0.0f));

    // System
    params.push_back(std::make_unique<juce::AudioParameterFloat>("gate", "Gate", 0.0f, 1.0f, 0.0f));

    return { params.begin(), params.end() };
}

void GeneSynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    
    engine.prepare(spec);
}

void GeneSynthAudioProcessor::releaseResources() {}

bool GeneSynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
}

void GeneSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    using namespace genesynth;

    
    // 1. Synchronisation APVTS -> OOP allParams
    auto safeUpdateStatic = [&](const char* idStr, genesynth::ParamID pid) {
        if (auto* p = apvts.getRawParameterValue(idStr)) getParamAs<genesynth::StaticParameter>(pid)->setValue(p->load());
    };
    auto safeUpdateMod = [&](const char* idStr, genesynth::ParamID pid) {
        if (auto* p = apvts.getRawParameterValue(idStr)) getParamAs<genesynth::ModulableParameter>(pid)->setBaseValue(p->load());
    };

    safeUpdateStatic("osc_a_wave", ParamID::OSC_A_WAVE);
    safeUpdateStatic("osc_a_ratio", ParamID::OSC_A_RATIO);
    safeUpdateMod("osc_a_vol_base", ParamID::OSC_A_VOL_BASE);
    safeUpdateStatic("osc_a_vol_mod", ParamID::OSC_A_VOL_MOD);
    safeUpdateMod("osc_a_freq_base", ParamID::OSC_A_FREQ_BASE);
    safeUpdateStatic("osc_a_freq_mod", ParamID::OSC_A_FREQ_MOD);
    safeUpdateMod("osc_a_fm_base", ParamID::OSC_A_FM_BASE);
    safeUpdateStatic("osc_a_fm_mod", ParamID::OSC_A_FM_MOD);

    safeUpdateStatic("osc_b_wave", ParamID::OSC_B_WAVE);
    safeUpdateStatic("osc_b_ratio", ParamID::OSC_B_RATIO);
    safeUpdateMod("osc_b_vol_base", ParamID::OSC_B_VOL_BASE);
    safeUpdateStatic("osc_b_vol_mod", ParamID::OSC_B_VOL_MOD);
    safeUpdateMod("osc_b_freq_base", ParamID::OSC_B_FREQ_BASE);
    safeUpdateStatic("osc_b_freq_mod", ParamID::OSC_B_FREQ_MOD);

    safeUpdateStatic("noise_type", ParamID::NOISE_TYPE);
    safeUpdateMod("noise_vol_base", ParamID::NOISE_VOL_BASE);
    safeUpdateStatic("noise_vol_mod", ParamID::NOISE_VOL_MOD);
    safeUpdateMod("noise_freq_base", ParamID::NOISE_FREQ_BASE);
    safeUpdateStatic("noise_freq_mod", ParamID::NOISE_FREQ_MOD);

    safeUpdateStatic("filter_type", ParamID::FILTER_TYPE);
    safeUpdateStatic("filter_res", ParamID::FILTER_RES);
    safeUpdateMod("filter_cutoff_base", ParamID::FILTER_CUTOFF_BASE);
    safeUpdateStatic("filter_cutoff_mod", ParamID::FILTER_CUTOFF_MOD);
    safeUpdateMod("filter_mix_base", ParamID::FILTER_MIX_BASE);
    safeUpdateStatic("filter_mix_mod", ParamID::FILTER_MIX_MOD);

    safeUpdateStatic("env_mod_a", ParamID::ENV_MOD_A);
    safeUpdateStatic("env_mod_d", ParamID::ENV_MOD_D);
    safeUpdateStatic("env_mod_s", ParamID::ENV_MOD_S);
    safeUpdateStatic("env_mod_r", ParamID::ENV_MOD_R);
    safeUpdateStatic("lfo_amt", ParamID::LFO_AMT);
    safeUpdateStatic("lfo_freq", ParamID::LFO_FREQ);

    safeUpdateStatic("env_global_a", ParamID::ENV_GLOBAL_A);
    safeUpdateStatic("env_global_d", ParamID::ENV_GLOBAL_D);
    safeUpdateStatic("env_global_s", ParamID::ENV_GLOBAL_S);
    safeUpdateStatic("env_global_r", ParamID::ENV_GLOBAL_R);

    safeUpdateStatic("clipper_vol", ParamID::CLIPPER_VOL);
    safeUpdateStatic("clipper_thresh", ParamID::CLIPPER_THRESH);
    safeUpdateStatic("reverb_decay", ParamID::REVERB_DECAY);
    safeUpdateStatic("reverb_size", ParamID::REVERB_SIZE);
    safeUpdateStatic("reverb_mix", ParamID::REVERB_MIX);

    safeUpdateStatic("gate", ParamID::GATE);

    // Clear outputs
    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    if (midiMessages.getNumEvents() > 0) {
        std::cout << "processBlock: received " << midiMessages.getNumEvents() << " MIDI events from Host!\n";
    }

    // HACK FOR TESTING: Translate GATE parameter to MIDI Note On/Off
    // DawDreamer MIDI seems to be ignored, so we generate it internally for tests
    static bool wasGate = false;
    bool isGate = getParamAs<StaticParameter>(ParamID::GATE)->getValue() >= 0.5f;
    if (isGate && !wasGate) {
        midiMessages.addEvent(juce::MidiMessage::noteOn(1, 69, (juce::uint8)127), 0);
    } else if (!isGate && wasGate) {
        midiMessages.addEvent(juce::MidiMessage::noteOff(1, 69, (juce::uint8)0), 0);
    }
    wasGate = isGate;

    // 2. Traitement Audio OOP
    engine.getPolyphonyManager().setMidiBuffer(&midiMessages);
    
    juce::dsp::AudioBlock<float> audioBlock(buffer);
    juce::dsp::ProcessContextReplacing<float> context(audioBlock);
    
    engine.process(context);
}

bool GeneSynthAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* GeneSynthAudioProcessor::createEditor() { return new GeneSynthAudioProcessorEditor(*this); }
const juce::String GeneSynthAudioProcessor::getName() const { return JucePlugin_Name; }
bool GeneSynthAudioProcessor::acceptsMidi() const { return true; }
bool GeneSynthAudioProcessor::producesMidi() const { return false; }
bool GeneSynthAudioProcessor::isMidiEffect() const { return false; }
double GeneSynthAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int GeneSynthAudioProcessor::getNumPrograms() { return 1; }
int GeneSynthAudioProcessor::getCurrentProgram() { return 0; }
void GeneSynthAudioProcessor::setCurrentProgram (int) {}
const juce::String GeneSynthAudioProcessor::getProgramName (int) { return {}; }
void GeneSynthAudioProcessor::changeProgramName (int, const juce::String&) {}
void GeneSynthAudioProcessor::getStateInformation (juce::MemoryBlock&) {}
void GeneSynthAudioProcessor::setStateInformation (const void*, int) {}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GeneSynthAudioProcessor();
}
