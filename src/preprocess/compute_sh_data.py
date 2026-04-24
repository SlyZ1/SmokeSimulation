import numpy as np
from scipy.special import legendre
from sympy.physics.wigner import gaunt

def unit_rbf(x):
    if not np.isscalar(x):
        x = np.linalg.norm(x)
    return np.exp(-x**2)

def optical_depth(beta=None, b2=None):
    # intergrale  exp(-(t² + b²)) dt
    # = exp(-b²) integrale exp(-t²) <---- gaussian integral
    # = sqrt(pi) exp(-b²)
    # b = sin(beta)
    if beta is None and b2 is None:
        raise ValueError("Both beta and b2 are None.")
    if beta is not None and b2 is not None:
        raise ValueError("Only beta or b2 should be entered.")
    if beta is not None:
        b2 = np.sin(beta)**2
    return np.sqrt(np.pi) * np.exp(-b2)

def compute_zh_coeffs(order=4, n_samples=512):
    mu = np.linspace(-1, 1, n_samples)
    T = optical_depth(b2=(1 - mu**2))
    coeffs = []
    for l in range(order):
        if l % 2 == 1:
            coeffs.append(0)
            continue
        P_l = legendre(l)(mu)
        integrand = T * P_l
        c_l = (2*l + 1) / 2 * np.trapezoid(integrand, mu)
        coeffs.append(c_l)
    return np.array(coeffs)

def ZH_optical_depth(beta, order=4):
    mu = np.cos(beta)
    result = 0.0
    for l in range(order):
        result += coeffs[l] * legendre(l)(mu)
    return result

def lm_to_idx(l, m):
    return l*l + l + m

def compute_gaunt_tensor(order=4):
    n = order**2
    tensor = np.zeros((n, n, n))
    for l1 in range(order):
        for m1 in range(-l1, l1+1):
            for l2 in range(order):
                for m2 in range(-l2, l2+1):
                    for l3 in range(order):
                        for m3 in range(-l3, l3+1):
                            if (l1 + l2 + l3) % 2 != 0:
                                continue
                            if (m1 + m2 + m3) > 0:
                                continue
                            if l3 < abs(l1-l2) or l3 > l1+l2:
                                continue

                            i = lm_to_idx(l1, m1)
                            j = lm_to_idx(l2, m2)
                            k = lm_to_idx(l3, m3)
                            tensor[i,j,k] = float(gaunt(l1, l2, l3, m1, m2, m3))
    return tensor

def generate_triple_product_glsl(tensor, order=4):
    n = order**2
    lines = []
    lines.append("void sh_triple_product3(float f[16], float g[16], out float result[16]){\n")
    for i in range(n):
        for j in range(n):
            for k in range(n):
                v = tensor[i,j,k]
                if abs(v) > 1e-10:
                    lines.append(f"\tresult[{i}] += {v:.6f} * f[{j}] * g[{k}];\n")
    lines.append("}")
    with open("sh_triple_product.glsl", "w") as f:
        f.writelines(lines)

if __name__ == "__main__":
    order = 4

    # ZH COEFFS
    coeffs = compute_zh_coeffs(order=order, n_samples=10000)
    coeffs.tofile("zh.bin")
    print(f"Zonal coefficients for order {order}: \n{coeffs}")
    # betas = np.linspace(0, np.pi, 6)
    # print(ZH_optical_depth(betas))
    # print(optical_depth(betas))

    # GAUNT TENSOR
    gamma = compute_gaunt_tensor()
    generate_triple_product_glsl(gamma)
    print("Gamma written in glsl file.")