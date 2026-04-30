import numpy as np

params = np.load("raw_params.npz")
C = params['C']
R = params['R']
W = params['W']

print("C:")
print(f"\t• {np.min(C[:,0])} {np.min(C[:,1])} {np.min(C[:,2])} - {np.max(C[:,0])} {np.max(C[:,1])} {np.max(C[:,2])}")
print(f"\t• {np.mean(C, axis=0)}")

print("R:")
print(f"\t• {np.min(R)} - {np.max(R)}")
print(f"\t• {np.mean(R, axis=0)}")

print("W:")
print(f"\t• {np.min(W)} - {np.max(W)}")
print(f"\t• {np.mean(W, axis=0)}")

params_bin = np.concatenate([C.flatten(), R, W])
params_bin.tofile("rbfs.bin")