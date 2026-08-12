import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import DataLoader, TensorDataset, random_split
import os

from models.cnn_model import GeneSynthNet

def train_model(dataset_path, epochs=10, batch_size=32, learning_rate=0.001, output_onnx="model.onnx", architecture="CNN"):
    print(f"Loading dataset from {dataset_path}...")
    dataset_dict = torch.load(dataset_path)
    X = dataset_dict['features']
    Y = dataset_dict['labels']
    
    num_params = Y.shape[1]
    print(f"Loaded dataset: X={X.shape}, Y={Y.shape}")
    
    dataset = TensorDataset(X, Y)
    val_size = int(0.2 * len(dataset))
    train_size = len(dataset) - val_size
    train_dataset, val_dataset = random_split(dataset, [train_size, val_size])
    
    train_loader = DataLoader(train_dataset, batch_size=batch_size, shuffle=True)
    val_loader = DataLoader(val_dataset, batch_size=batch_size, shuffle=False)
    
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print(f"Using device: {device}")
    
    model = GeneSynthNet(num_params=num_params).to(device)
    criterion = nn.MSELoss()
    optimizer = optim.Adam(model.parameters(), lr=learning_rate)
    
    best_val_loss = float('inf')
    best_model_path = os.path.join(os.path.dirname(output_onnx), "best_model.pth")
    
    for epoch in range(epochs):
        model.train()
        train_loss = 0.0
        for batch_X, batch_Y in train_loader:
            batch_X, batch_Y = batch_X.to(device), batch_Y.to(device)
            
            optimizer.zero_grad()
            outputs = model(batch_X)
            loss = criterion(outputs, batch_Y)
            loss.backward()
            optimizer.step()
            
            train_loss += loss.item() * batch_X.size(0)
            
        train_loss /= len(train_loader.dataset)
        
        model.eval()
        val_loss = 0.0
        with torch.no_grad():
            for batch_X, batch_Y in val_loader:
                batch_X, batch_Y = batch_X.to(device), batch_Y.to(device)
                outputs = model(batch_X)
                loss = criterion(outputs, batch_Y)
                val_loss += loss.item() * batch_X.size(0)
                
        val_loss /= len(val_loader.dataset)
        
        print(f"Epoch {epoch+1}/{epochs} | Train Loss: {train_loss:.4f} | Val Loss: {val_loss:.4f}")
        
        if val_loss < best_val_loss:
            best_val_loss = val_loss
            torch.save(model.state_dict(), best_model_path)
            
    print("Training complete. Loading best model for ONNX export...")
    model.load_state_dict(torch.load(best_model_path))
    model.eval()
    
    # Export to ONNX
    dummy_input = torch.randn(1, 86, 128).to(device)
    torch.onnx.export(
        model, 
        dummy_input, 
        output_onnx, 
        export_params=True,
        opset_version=11,
        do_constant_folding=True,
        input_names=['input'], 
        output_names=['output'],
        dynamic_axes={'input': {0: 'batch_size'}, 'output': {0: 'batch_size'}}
    )
    print(f"Model exported to ONNX format at {output_onnx}")

if __name__ == "__main__":
    import sys
    dataset_path = sys.argv[1] if len(sys.argv) > 1 else "dataset_test.pt"
    train_model(dataset_path)
