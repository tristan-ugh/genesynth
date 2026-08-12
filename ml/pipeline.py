import yaml
import json
import argparse
import os

from data.generate_dataset import generate
from scripts.train import train_model

def load_config(config_path):
    with open(config_path, 'r') as f:
        if config_path.endswith('.yaml') or config_path.endswith('.yml'):
            return yaml.safe_load(f)
        elif config_path.endswith('.json'):
            return json.load(f)
        else:
            raise ValueError("Unsupported config file format. Use .json or .yaml")

def main():
    default_config = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'configs', 'pipeline_config.yaml')
    parser = argparse.ArgumentParser(description="GeneSynth Global Pipeline")
    parser.add_argument('--config', type=str, default=default_config, help='Path to configuration file')
    args = parser.parse_args()
    
    if not os.path.exists(args.config):
        print(f"Error: Configuration file {args.config} not found.")
        return
        
    config = load_config(args.config)
    
    if 'pipeline' not in config:
        print("Error: 'pipeline' key not found in configuration.")
        return
        
    for i, step_config in enumerate(config['pipeline']):
        step_name = step_config.get('step')
        print(f"\n{'='*40}")
        print(f"Executing Pipeline Step {i+1}: {step_name}")
        print(f"{'='*40}")
        
        if step_name == 'generate_dataset':
            mode = step_config.get('mode', 'synthetic')
            if mode == 'synthetic':
                num_samples = step_config.get('num_samples', 100)
                output_path = step_config.get('output_path', 'dataset.pt')
                generate(num_samples=num_samples, output_path=output_path)
            else:
                print(f"Mode {mode} not yet supported for generate_dataset.")
                
        elif step_name == 'train_model':
            dataset_path = step_config.get('dataset_path', 'dataset.pt')
            epochs = step_config.get('epochs', 10)
            batch_size = step_config.get('batch_size', 32)
            learning_rate = step_config.get('learning_rate', 0.001)
            output_onnx = step_config.get('output_onnx', 'model.onnx')
            architecture = step_config.get('architecture', 'CNN')
            
            if not os.path.exists(dataset_path):
                print(f"Error: Dataset {dataset_path} not found. Ensure generation step ran successfully.")
                break
                
            train_model(
                dataset_path=dataset_path, 
                epochs=epochs, 
                batch_size=batch_size, 
                learning_rate=learning_rate, 
                output_onnx=output_onnx,
                architecture=architecture
            )
            
        else:
            print(f"Unknown step: {step_name}")

if __name__ == "__main__":
    main()
