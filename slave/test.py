import calculator
import socket

import torch
import torch.nn as nn
import torch.nn.functional as F
from torch.utils.data import DataLoader, TensorDataset, random_split

import torch.optim as optim
import torch.distributions as dist

import matplotlib.pyplot as plt
from sklearn.metrics import confusion_matrix, ConfusionMatrixDisplay, classification_report
import numpy as np

import pandas as pd
import math

import torch.distributions as dist

import numpy as np

X=None
y=None

class MulticlassClassifier(nn.Module):
    def __init__(self, input_dim: int, num_classes: int, hidden1: int = 128, hidden2: int = 64,hidden3: int = 32):
        super(MulticlassClassifier, self).__init__()
        self.fc1 = nn.Linear(input_dim, hidden1)
        self.fc2 = nn.Linear(hidden1, hidden2)
        self.fc3 = nn.Linear(hidden2, hidden3)
        
        self.class_logits = nn.Linear(hidden3, num_classes)      # Predict class scores
        self.class_log_vars = nn.Linear(hidden3, num_classes)     # Predict log-variance for each class

    def forward(self, x: torch.Tensor):
        x = F.relu(self.fc1(x))
        x = F.relu(self.fc2(x))
        x = F.relu(self.fc3(x))
        
        logits = self.class_logits(x)
        log_vars = self.class_log_vars(x)
        return logits, log_vars
        #return logits, logits


        
     
 # Add two numbers    #EJEMPLO DEL PROFE
#result = calculator.add(5.0, 3.0)
#print(result)  # Output: 8.0

# Subtract two numbers EJEMPLO DEL PROFE
#result = calculator.subtract(10.0, 4.0)
#print(result)  # Output: 6.0

# Multiply two numbers EJEMPLO DEL PROFE
#result = calculator.multiply(6.0, 7.0)
#print(result)  # Output: 42.0

# Divide two numbers EJEMPLO DEL PROFE
#result = calculator.divide(15.0, 3.0)
#print(result)  # Output: 5.0

# Divide two numbers EJEMPLO DEL PROFE
#result = calculator.fill(8, 30)
#print(result)  # Output: 5.0

#-----------------------------------------------------INICIO-------------------------------------------------------------------------

PORT = 9000

#-----------------------------------------------------Crea estructura cliente con socket
client = calculator.UDPClient("192.168.107.107", 9000)

#----------------------------------------------------- envia un mensaje de login
protocol = client.buildProtocol('L', "")
storage = calculator.split(protocol)
client.send_data(storage, 0)

#-----------------------------------------------------recibe las 3 matrices y el conjunto de datos en total 4 cosas    
for i in range(0,4):
    client.fread();


# Generate synthetic heteroscedastic multiclass data
torch.manual_seed(42)
num_samples = 1000
input_dim = 14
num_classes = 3
batch_size = 1

X = torch.randn(num_samples, input_dim)  # 1000 samples, 100 inputs

active_indices = torch.randint(0, num_classes, (num_samples,))
y = torch.nn.functional.one_hot(active_indices, num_classes=num_classes).float()

#y = torch.randint(0, num_classes, (num_samples * num_classes,)).view(num_samples, num_classes)

# Load dataset from CSV
csv_path = "Dataset of Diabetes.csv"  # replace with your actual CSV file path   1000,13,3,50

df = pd.read_csv(csv_path,header=None, skiprows=1)


# Assume first 4 columns are input features, last 3 are one-hot class labels
X_np =        df.iloc[:, :input_dim].values.astype(np.float32)
#y_onehot_np = df.iloc[:, input_dim:].values.astype(np.float32)
y_onehot_np = df.iloc[:, -num_classes:].values.astype(np.float32)

#y_np = np.argmax(y_onehot_np, axis=1).astype(np.int64)


X = torch.tensor(X_np)
y = torch.tensor(y_onehot_np)
print("y ",y)
print("y ",y.size())
print("X ",X.size())
# Create dataset and split into training/testing
dataset = TensorDataset(X, y)

train_size = int(0.8 * len(dataset))
test_size = len(dataset) - train_size
train_dataset, test_dataset = random_split(dataset, [train_size, test_size])

train_loader = DataLoader(train_dataset, batch_size=batch_size, shuffle=True)
test_loader = DataLoader(test_dataset, batch_size=batch_size, shuffle=False)

# Model setup
model = MulticlassClassifier(input_dim=input_dim, num_classes=num_classes)



#-----------------------------------------------------CARGA DE LOS PESOS que se mandaron antes
pesos1 = np.loadtxt("pesos_fc1.txt")
pesos2 = np.loadtxt("pesos_fc2.txt")
pesos3 = np.loadtxt("pesos_fc3.txt")

model.fc1.weight.data.copy_(
    torch.tensor(pesos1, dtype=torch.float32)
)

model.fc2.weight.data.copy_(
    torch.tensor(pesos2, dtype=torch.float32)
)

model.fc3.weight.data.copy_(
    torch.tensor(pesos3, dtype=torch.float32)
)




criterion = nn.CrossEntropyLoss()
optimizer = torch.optim.Adam(model.parameters(), lr=0.001)

# Training loop
num_epochs = 1
train_tracker, test_tracker, accuracy_tracker = [], [], []

# Evaluation on test set
model.eval()
y_true = []
y_pred = []
log_vars_all = []

for epoch in range(num_epochs):
    model.train()
    epoch_loss = 0
    for batch_x, batch_y in train_loader:
        optimizer.zero_grad()
        logits, log_vars = model(batch_x)
        loss = criterion(logits, batch_y)
        loss.backward()
        optimizer.step()
        epoch_loss += loss.item()
    train_tracker.append(epoch_loss / len(train_loader))
    print(f"Epoch {epoch+1}/{num_epochs}, Loss: {train_tracker[-1]:.4f} | ",end="")



#with torch.no_grad():
    test_loss = 0
    total = 0
    num_correct = 0
    for batch_x, batch_y in test_loader:
        logits, log_vars = model(batch_x)
        loss = criterion(logits, batch_y)
        test_loss += loss.item()

        predictions = torch.argmax(logits, dim=1)
        total += batch_x.size(0)
        num_correct += (predictions == torch.argmax(batch_y, dim=1)).sum().item()

        predictions = torch.argmax(logits, dim=1)
        y_true.extend(torch.argmax(batch_y, dim=1))
        y_pred.extend(predictions.tolist())
        log_vars_all.append(log_vars)

    test_tracker.append(test_loss/len(test_loader))
    print(f"Test loss: {test_loss/len(test_loader)} | ", end='')
    accuracy_tracker.append(num_correct/total)
    print(f'Accuracy : {num_correct/total}')


#-----------------------------------------------------GUARDAR PESOS en txts despues de hacer el entrenamiento
weight1 = np.matrix(model.fc1.weight.detach().cpu().numpy())
np.savetxt("pesos_fc1.txt", weight1)
weight2 = np.matrix(model.fc2.weight.detach().cpu().numpy())
np.savetxt("pesos_fc2.txt", weight2)
weight3 = np.matrix(model.fc3.weight.detach().cpu().numpy())
np.savetxt("pesos_fc3.txt", weight3)


#-----------------------------------------------------Manda los pesos a master
for i in range(1, 4):
    with open(f"pesos_fc{i}.txt", "r") as f:
        contenido = f.read()

    protocol = client.buildProtocol('M', contenido)
    storage = calculator.split(protocol)
    client.send_data(storage, 0)



# Plot training loss over epochs
plt.figure(figsize=(8, 4))
plt.plot(train_tracker, marker='o')
plt.title("Training Loss Over Epochs")
plt.xlabel("Epoch")
plt.ylabel("Loss")
plt.grid(True)
plt.tight_layout()
plt.show()

import matplotlib.pyplot as plt
# %matplotlib inline
plt.plot(train_tracker, label='Training loss')
plt.plot(test_tracker, label='Test loss')
plt.plot(accuracy_tracker, label='Test accuracy')
plt.legend()


# Display confusion matrix
cm = confusion_matrix(y_true, y_pred)
disp = ConfusionMatrixDisplay(confusion_matrix=cm, display_labels=list(range(num_classes)))
disp.plot(cmap=plt.cm.Blues)
plt.title("Confusion Matrix")
plt.tight_layout()
plt.show()

# Print classification metrics
print("\nClassification Report:")
print(classification_report(y_true, y_pred, digits=3))



