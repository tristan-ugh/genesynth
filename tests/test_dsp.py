import os
import pytest
import numpy as np
import dawdreamer as daw
from scipy.io import wavfile

SAMPLE_RATE = 44100
BUFFER_SIZE = 512
PLUGIN_PATH = os.path.abspath("build/genesynth_prototype_artefacts/VST3/GeneSynth.vst3")
OUTPUT_DIR = "tests/output"

os.makedirs(OUTPUT_DIR, exist_ok=True)

def save_trace(name, audio):
    audio_int16 = np.int16(np.clip(audio, -1.0, 1.0) * 32767)
    wavfile.write(os.path.join(OUTPUT_DIR, f"{name}.wav"), SAMPLE_RATE, audio_int16.T)

@pytest.fixture
def synth_engine():
    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    synth = engine.make_plugin_processor("genesynth", PLUGIN_PATH)
    
    # Mapped directly to what is exposed in PluginProcessor.cpp
    param_map = {
        "osc_a_wave": 0,
        "osc_a_ratio": 1,
        "osc_a_vol": 2,
        "noise_vol": 3,
        "filter_cutoff": 4,
        "env_global_a": 5,
        "env_global_d": 6,
        "env_global_s": 7,
        "env_global_r": 8,
        "gate": 9
    }

    def set_param_fn(name, value):
        if name in param_map:
            synth.set_parameter(param_map[name], value)
        else:
            raise ValueError(f"Parametre non trouvé dans param_map : {name}")

    # Reset default parameters to silent/neutral state
    set_param_fn("osc_a_wave", 0.0)
    # osc_a_ratio is not set here because 1.0 normalized means max value (8.0)
    set_param_fn("osc_a_vol", 0.0) 
    set_param_fn("noise_vol", 0.0)
    set_param_fn("filter_cutoff", 1.0) 
    
    set_param_fn("env_global_a", 0.0)
    set_param_fn("env_global_d", 0.0)
    set_param_fn("env_global_s", 1.0)
    set_param_fn("env_global_r", 0.0)
    
    set_param_fn("gate", 0.0)
    
    return engine, synth, set_param_fn

def test_sine_simple(synth_engine):
    engine, synth, set_param = synth_engine
    set_param("osc_a_vol", 1.0)
    set_param("osc_a_wave", 0.0)
    set_param("gate", 1.0)
    
    # We must trigger a MIDI note to give the Voice a frequency!
    synth.add_midi_note(69, 127, 0.0, 1.0) # Note A4 (440Hz), vel 127, start 0.0s, duration 1.0s
    
    engine.load_graph([(synth, [])])
    engine.render(1.0)
    
    audio = engine.get_audio()[0]
    save_trace("trace_test_sine_simple", engine.get_audio())
    
    fft_spectrum = np.abs(np.fft.rfft(audio))
    freqs = np.fft.rfftfreq(len(audio), 1.0/SAMPLE_RATE)
    peak_freq = freqs[np.argmax(fft_spectrum)]
    
    assert 435.0 <= peak_freq <= 445.0, f"Fréquence inattendue : {peak_freq} Hz"
    
def test_saw_simple(synth_engine):
    engine, synth, set_param = synth_engine
    set_param("osc_a_vol", 1.0)
    set_param("osc_a_wave", 1.0)
    set_param("gate", 1.0)
    
    synth.add_midi_note(69, 127, 0.0, 1.0)
    
    engine.load_graph([(synth, [])])
    engine.render(1.0)
    
    audio = engine.get_audio()[0]
    save_trace("trace_test_saw_simple", engine.get_audio())
    
    fft_spectrum = np.abs(np.fft.rfft(audio))
    freqs = np.fft.rfftfreq(len(audio), 1.0/SAMPLE_RATE)
    fond_energy = fft_spectrum[np.argmax(fft_spectrum)]
    high_freq_energy = np.sum(fft_spectrum[freqs > 500.0])
    
    assert high_freq_energy / fond_energy > 0.5, "Pas assez d'harmoniques pour une Sawtooth"

def test_noise_simple(synth_engine):
    engine, synth, set_param = synth_engine
    set_param("noise_vol", 1.0)
    set_param("gate", 1.0)
    
    synth.add_midi_note(69, 127, 0.0, 1.0)
    
    engine.load_graph([(synth, [])])
    engine.render(1.0)
    
    audio = engine.get_audio()[0]
    save_trace("trace_test_noise_simple", engine.get_audio())
    rms = np.sqrt(np.mean(audio**2))
    
    assert rms > 0.05, "Le bruit n'a pas été généré."

def test_envelope_volume(synth_engine):
    engine, synth, set_param = synth_engine
    set_param("osc_a_vol", 1.0)
    set_param("env_global_a", 0.4) # ~0.5s attaque
    set_param("gate", 1.0)
    
    synth.add_midi_note(69, 127, 0.0, 1.0)
    
    engine.load_graph([(synth, [])])
    engine.render(1.0)
    
    audio = engine.get_audio()[0]
    save_trace("trace_test_envelope_volume", engine.get_audio())
    
    rms_early = np.sqrt(np.mean(audio[int(0.1 * SAMPLE_RATE):int(0.2 * SAMPLE_RATE)]**2))
    rms_late = np.sqrt(np.mean(audio[int(0.8 * SAMPLE_RATE):int(0.9 * SAMPLE_RATE)]**2))
    
    assert rms_early < rms_late, "Le volume ne monte pas progressivement."
