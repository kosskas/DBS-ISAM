import pickle as pk
import random
import struct

#wykladniki = [4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24]
wykladniki = [3]
maxN = 16000
k = 1
for i in wykladniki:
    n_rekordow = 2**i
    zakres = list(range(1, 4))
    #maxN = maxN*2
    rekordy = [(i+1)*10 for i in range(2*n_rekordow)]
    
    with open(f"idx{k}", "wb") as f:
        for r in rekordy:           
            f.write(struct.pack('<I', r))
    print(f"Zapisano do idx{k}")
    k += 1
