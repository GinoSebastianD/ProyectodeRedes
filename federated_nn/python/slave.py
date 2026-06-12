import argparse
import sys
import numpy as np
import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim

try:
    import comm_module
except ImportError:
    print("ERROR: comm_module no encontrado.")
    print("  cd cpp && python setup.py build_ext --inplace")
    print("  cp comm_module*.so ../python/")
    sys.exit(1)


#Configuración
MASTER_HOST     = "127.0.0.1"
MASTER_PORT     = 9000

INPUT_DIM       = 14
NUM_CLASSES     = 3
HIDDEN1         = 128
HIDDEN2         = 64
LR              = 0.001

RECV_TIMEOUT_MS = 120_000   



class MulticlassClassifier(nn.Module):
    def __init__(self):
        super().__init__()
        self.fc1            = nn.Linear(INPUT_DIM, HIDDEN1)
        self.fc2            = nn.Linear(HIDDEN1, HIDDEN2)
        self.class_logits   = nn.Linear(HIDDEN2, NUM_CLASSES)
        self.class_log_vars = nn.Linear(HIDDEN2, NUM_CLASSES)

    def forward(self, x):
        x = F.relu(self.fc1(x))
        x = F.relu(self.fc2(x))
        return self.class_logits(x), self.class_log_vars(x)



def weights_to_bytes(model: nn.Module) -> bytes:
    return np.concatenate([
        p.data.cpu().numpy().astype(np.float32).flatten()
        for p in model.parameters()
    ]).tobytes()


def bytes_to_weights(model: nn.Module, data: bytes) -> None:
    flat = np.frombuffer(data, dtype=np.float32).copy()
    ptr  = 0
    with torch.no_grad():
        for param in model.parameters():
            n = param.numel()
            param.data.copy_(
                torch.from_numpy(flat[ptr:ptr + n].reshape(param.shape)))
            ptr += n



def deserialize_dataset(data: bytes):
    """
    Retorna (X, y) como numpy arrays float32.
    X.shape = [n_samples, n_features]
    y.shape = [n_samples, n_classes]
    """
    hdr     = np.frombuffer(data[:12], dtype=np.uint32)
    n, f, c = int(hdr[0]), int(hdr[1]), int(hdr[2])

    off_x = 12
    off_y = off_x + n * f * 4
    X = np.frombuffer(data[off_x:off_y],
                      dtype=np.float32).reshape(n, f).copy()
    y = np.frombuffer(data[off_y:off_y + n * c * 4],
                      dtype=np.float32).reshape(n, c).copy()
    return X, y



#Función principal

def main():
    parser = argparse.ArgumentParser(description="Esclavo – Federated Learning")
    parser.add_argument("--node-id",    type=int, required=True,
                        help="ID de este esclavo (1, 2, 3 …)")
    parser.add_argument("--port",       type=int, required=True,
                        help="Puerto UDP de escucha")
    parser.add_argument("--max-rounds", type=int, default=10000,
                        help="Máximo de rondas (momentos)")
    args    = parser.parse_args()
    nid     = args.node_id
    port    = args.port

    sep = "=" * 50
    print(sep)
    print(f"  ESCLAVO {nid} – Aprendizaje Federado (batch=1)")
    print(sep)
    print(f"  Puerto escucha : {port}  (node_id={nid})")
    print(f"  Maestro        : {MASTER_HOST}:{MASTER_PORT}")
    print()

  
    node = comm_module.RDTNode(port, nid, print)

    model     = MulticlassClassifier()
    criterion = nn.CrossEntropyLoss()
    optimizer = optim.Adam(model.parameters(), lr=LR)


    # recibir porción del dataset
 
    print(sep)
    print(f"  [Esclavo {nid}] Esperando dataset del maestro (TYPE=D)...")
    print(sep)

    result = node.recv_any(RECV_TIMEOUT_MS)
    if result is None:
        print(f"[Esclavo {nid}] ERROR: Timeout esperando dataset. Abortando.")
        return

    tipo, raw = result
    if tipo != 'D':
        print(f"[Esclavo {nid}] ERROR: Esperaba TYPE='D', llegó '{tipo}'. Abortando.")
        return

    X_local, y_local = deserialize_dataset(raw)
    n_momentos = len(X_local)
    print(f"[Esclavo {nid}]  Dataset recibido: {n_momentos} filas "
          f"({X_local.shape[1]} features, {y_local.shape[1]} clases)")


    X_t = torch.tensor(X_local)   
    y_t = torch.tensor(y_local)  


    for momento in range(min(n_momentos, args.max_rounds)):
        print(f"\n{sep}")
        print(f"  [Esclavo {nid}] MOMENTO {momento + 1} / {n_momentos}")
        print(sep)

        #Recibir matriz de pesos del maestro (TYPE = 'M')
        print(f"[Esclavo {nid}] ← Esperando pesos (TYPE=M) del maestro...")
        result = node.recv_any(RECV_TIMEOUT_MS)

        if result is None:
            print(f"[Esclavo {nid}] Timeout — fin del entrenamiento")
            break

        tipo, raw = result
        if tipo != 'M':
            print(f"[Esclavo {nid}] ERROR: Esperaba TYPE='M', llegó '{tipo}'")
            break

        #Cargar pesos en el modelo local 
        bytes_to_weights(model, raw)
        print(f"[Esclavo {nid}]  Pesos del maestro cargados ({len(raw)} B)")

        #Entrenar con la FILA `momento` (batch = 1)
        sx = X_t[momento].unsqueeze(0)   
        sy = y_t[momento].unsqueeze(0)   

        model.train()
        optimizer.zero_grad()
        logits, _ = model(sx)
        loss = criterion(logits, sy)
        loss.backward()           # BACKPROPAGATION
        optimizer.step()

        pred  = logits.argmax(dim=1).item()
        label = sy.argmax(dim=1).item()
        print(f"[Esclavo {nid}] Entrenamiento fila {momento+1}: "
              f"loss={loss.item():.4f}  pred={pred}  label={label}")

        #Enviar pesos actualizados al maestro (TYPE = 'm') 
        updated = weights_to_bytes(model)
        print(f"[Esclavo {nid}] - Enviando pesos (TYPE=m) al maestro "
              f"({len(updated)} B)...")
        try:
            node.send_matrix_slave(MASTER_HOST, MASTER_PORT, 0, updated)
            print(f"[Esclavo {nid}]  Pesos enviados al maestro")
        except RuntimeError as e:
            print(f"[Esclavo {nid}]  Error enviando: {e}")

    print(f"\n[Esclavo {nid}] Entrenamiento completado "
          f"({min(n_momentos, args.max_rounds)} momentos).")


if __name__ == "__main__":
    main()
