# # Observe GPU Usage using nvidia-smi -l 1 
import torch
import time

device = "cuda"

x = torch.randn(12000, 12000, device=device)
y = torch.randn(12000, 12000, device=device)

torch.cuda.synchronize()

start = time.time()

for _ in range(20):
    z = x @ y

torch.cuda.synchronize()

print("Time:", time.time()-start)
print("GPU:", torch.cuda.get_device_name(0))
print("Memory GB:", torch.cuda.memory_allocated()/1e9)