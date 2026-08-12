import torch
import numpy as np
import random
import os
from genesynth import GeneSynth
import time

def generate_constrained_preset(synth):
    """
    Generates a dictionary of parameters that are "not too random"
    by enforcing musical or logical constraints.
    """
    params = {}
    
    # 1. Oscillators: mostly use basic waveforms (0.0, 0.5, 1.0)
    params["osc_a_wave"] = random.choice([0.0, 0.5, 1.0])
    params["osc_b_wave"] = random.choice([0.0, 0.5, 1.0])
    
    # 2. Volumes: Ensure at least one oscillator is loud
    if random.random() > 0.5:
        params["osc_a_vol_base"] = random.uniform(0.7, 1.0)
        params["osc_b_vol_base"] = random.uniform(0.0, 0.5)
    else:
        params["osc_a_vol_base"] = random.uniform(0.0, 0.5)
        params["osc_b_vol_base"] = random.uniform(0.7, 1.0)
        
    # 3. Filter: Cutoff should mostly be open, but sometimes closed for bass
    # Skew the distribution towards higher values
    params["filter_cutoff_base"] = min(1.0, max(0.0, random.gauss(0.8, 0.2)))
    params["filter_res"] = random.uniform(0.0, 0.7) # rarely self-oscillate
    
    # 4. Envelopes: Plucky sounds vs Pads
    is_pad = random.random() > 0.7
    if is_pad:
        params["env_global_a"] = random.uniform(0.3, 0.8) # Slow attack
        params["env_global_r"] = random.uniform(0.4, 0.9) # Long release
    else:
        params["env_global_a"] = random.uniform(0.0, 0.1) # Fast attack
        params["env_global_r"] = random.uniform(0.0, 0.3) # Fast release
        
    # 5. FM and LFO: Keep it subtle most of the time
    params["osc_b_fm"] = random.uniform(0.0, 0.3)
    params["lfo_amt"] = random.uniform(0.0, 0.2)
    
    # Fill the rest with completely uniform randoms just in case
    for pid in synth.get_parameter_ids():
        if pid not in params:
            params[pid] = random.random()
            
    return params

import sys

def generate(num_samples=100, output_path="dataset_test.pt"):
    print("Loading GeneSynth Python API...")
    try:
        synth = GeneSynth()
    except Exception as e:
        print(f"Failed to load GeneSynth: {e}")
        print("Make sure you compiled the C++ project with the new target!")
        return
        
    print(f"GeneSynth loaded! Found {synth.num_params} parameters.")
    
    features_list = []
    labels_list = []
    
    print(f"Generating {num_samples} samples...")
    start_time = time.time()
    
    for i in range(num_samples):
        # 1. Generate parameters
        param_dict = generate_constrained_preset(synth)
        
        # 2. Render Audio and Extract Spectrogram
        # We can pass the dict directly; it handles the ordering!
        spectrogram = synth.render(params_dict=param_dict)
        
        # 3. Convert param_dict back to an array in the exact correct order for labels
        ordered_params = [param_dict.get(pid, 0.5) for pid in synth.get_parameter_ids()]
        
        features_list.append(spectrogram)
        labels_list.append(ordered_params)
        
        if (i+1) % 10 == 0:
            print(f"Generated {i+1}/{num_samples}...")
            
    end_time = time.time()
    print(f"Generation finished in {end_time - start_time:.2f} seconds.")
    
    # Convert to PyTorch Tensors
    X = torch.tensor(np.array(features_list), dtype=torch.float32)
    Y = torch.tensor(np.array(labels_list), dtype=torch.float32)
    
    print(f"Dataset Shapes:")
    print(f"Features (Spectrograms): {X.shape}  # Expected: [N, 86, 128]")
    print(f"Labels (Parameters):     {Y.shape}  # Expected: [N, 41]")
    
    # Save dataset
    torch.save({'features': X, 'labels': Y}, output_path)
    print(f"Saved to {output_path}")

def main():
    num_samples = 100
    if len(sys.argv) > 1:
        try:
            num_samples = int(sys.argv[1])
        except ValueError:
            print("Invalid number of samples provided.")
            return
    generate(num_samples=num_samples, output_path="dataset_test.pt")

if __name__ == "__main__":
    main()
