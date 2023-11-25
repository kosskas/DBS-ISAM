import pickle as pk
import random
import struct

#wykladniki = [4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24]
wykladniki = [4]
maxN = 16000
k = 1
for i in wykladniki:
    n_rekordow = 2**i
    zakres = list(range(1, 100))
    #maxN = maxN*2
    #rekordy = [(i+1)*10 for i in range(2*n_rekordow)]
    rekordy = [(i + 1) * 10 for i in range(n_rekordow)]
    ###FILE
    with open(f"file{k}", "wb") as f:
        for r in rekordy:
            rand_values = [random.choice(zakres) for _ in range(3)]
            rekord = [r] + rand_values + [0]  #
            f.write(struct.pack('<IIIII', *rekord))  
    print(f"Zapisano do file{k}")
    ##IDX
    i=0
    with open(f"idx{k}", "wb") as f:
        for r in rekordy:
            rekord = [r] + [i]
            f.write(struct.pack('<II', *rekord)) 
            i+=1
    print(f"Zapisano do idx{k}")
    k += 1
    

