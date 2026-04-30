import numpy as np
from scipy.special import legendre, sph_harm_y
from sympy.physics.wigner import gaunt
import matplotlib.pyplot as plt

# ----------- ZH COEFFS
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

def compute_zh_coeffs(order=4, n_beta=256):
    betas = np.linspace(0, np.pi, n_beta)
    table = np.zeros((n_beta, order))
    T = optical_depth(beta=betas)
    mu = np.cos(betas)
    for l in range(order):
        norm = np.sqrt((2*l + 1) / (4 * np.pi))
        table[:, l] = norm * T * legendre(l)(mu)
    print(table.flatten()[:8])
    table.flatten().astype(np.float32).tofile("zh.bin")
    

def ZH_optical_depth(beta, coeffs, order=4):
    mu = np.cos(beta)
    result = 0.0
    for l in range(order):
        result += coeffs[l] * legendre(l)(mu)
    return result

def lm_to_idx(l, m):
    return l*l + l + m

# -------------- GAUNT TRIPLE PROD

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
                            if (m1 + m2 + m3) != 0:
                                continue
                            if l3 < abs(l1-l2) or l3 > l1+l2:
                                continue

                            i = lm_to_idx(l1, m1)
                            j = lm_to_idx(l2, m2)
                            k = lm_to_idx(l3, m3)
                            val = gaunt(l1, l2, l3, m1, m2, m3)
                            tensor[i,j,k] = float(val)
    return tensor

def generate_triple_product_glsl(tensor, order=4):
    n = order**2
    lines = []
    lines.append("void sh_triple_product(float f[16], float g[16], out float result[16]){\n")
    equals = True
    for i in range(n):
        equals = True
        for j in range(n):
            for k in range(n):
                v = tensor[i,j,k]
                if abs(v) > 1e-10:
                    lines.append(f"\tresult[{i}] {'' if equals else '+'}= {v:.6f} * f[{j}] * g[{k}];\n")
                    equals = False
    lines.append("}")
    with open("sh_triple_product.glsl", "w") as f:
        f.writelines(lines)

# ------------------ EXP *
def compute_a_b_tables(max_magnitude=50, n_table=256, n_samples=1024, plot_res=False):
    t = np.linspace(0, 1, n_table)
    magnitudes = np.sqrt(t) * max_magnitude
    mu = np.linspace(-1, 1, n_samples)
    a_table = np.zeros(n_table)
    b_table = np.zeros(n_table)
    for i, mag in enumerate(magnitudes):
        f_vals = mag * mu
        exp_f = np.exp(f_vals)
        c0 = 0.5 * np.trapezoid(exp_f, mu)
        c1 = 1.5 * np.trapezoid(exp_f * mu, mu)

        # exp_star(f) ~= 1.a + f.b + f².c ...

        a = c0 / np.sqrt(4 * np.pi)
        b = c1 / mag if mag > 1e-8 else 1.0/3.0
        a_table[i] = a
        b_table[i] = b
    if plot_res:
        plt.plot(magnitudes, a_table, label='a')
        plt.plot(magnitudes, b_table, label='b')
        plt.show()
    exp_data = np.concatenate([[max_magnitude], a_table, b_table]).astype(np.float32)
    exp_data.tofile("exp_data.bin")

# ------------- SH CONV
def generate_sh_conv_glsl(order=4):
    lines = []
    lines.append("void sh_conv(float f[16], float g[16], out float result[16]){\n")
    equals = np.ones(order**2)
    for l in range(order):
        for m in range(-l, l+1):
            factor = np.sqrt(4 * np.pi / (2 * l + 1))
            i = lm_to_idx(l, m)
            j = lm_to_idx(l, 0)
            equal = equals[i]
            lines.append(f"\tresult[{i}] {'' if equal == 1 else '+'}= {factor:.6f} * f[{i}] * g[{j}];\n")
            equals[i] = 0
    lines.append("}")
    with open("sh_conv.glsl", "w") as f:
        f.writelines(lines)

# -------------- FUNC TO SH
def hg(g):
    factor = 1 / (4 * np.pi) * (1 - g**2)
    hg_func = lambda theta, _ : factor / np.pow(1 + g**2 - 2*g*np.cos(theta), 1.5)
    return hg_func

def directional_light(theta_l, phi_l, sharpness=1000):
    def f(theta, phi):
        mu = np.cos(theta)
        mu_l = np.cos(theta_l)
        # produit scalaire entre direction et lumière
        dot = mu * mu_l + np.sqrt(1-mu**2) * np.sqrt(1-mu_l**2) * np.cos(phi - phi_l)
        return np.exp(sharpness * (dot - 1))
    return f

def project_sh_full(f, order=4, n_theta=256, n_phi=512):
    u = np.linspace(-1, 1, n_theta)
    theta = np.arccos(u)
    phi = np.linspace(0, 2*np.pi, n_phi, endpoint=False)
    THETA, PHI = np.meshgrid(theta, phi, indexing='ij')
    dOmega = (2/n_theta) * (2*np.pi/n_phi)
    
    f_vals = f(THETA, PHI)
    
    coeffs = np.zeros(order**2)
    for l in range(order):
        for m in range(-l, l+1):
            i = lm_to_idx(l, m)
            Y = np.real(sph_harm_y(l, m, THETA, PHI))
            coeffs[i] = np.sum(f_vals * Y * dOmega)
    coeffs[np.abs(coeffs) < 1e-10] = 0
    return coeffs.astype(np.float32)

if __name__ == "__main__":
    order = 4

    # ZH COEFFS
    compute_zh_coeffs()

    # GAUNT TENSOR
    gamma = compute_gaunt_tensor()
    # generate_triple_product_glsl(gamma)
    print("Gamma written in glsl file.")

    # EXP *
    max_density_mag = 1.85
    sigma_t = 30
    max_mag = np.min([max_density_mag * sigma_t, 15])
    compute_a_b_tables(n_table=256, max_magnitude=max_mag, plot_res=False)
    print("Exp* parameters computed.")

    # SH CONV
    # generate_sh_conv_glsl()
    # print("SH conv written in glsl file.")

    # HG to SH
    hg_sh = project_sh_full(hg(0.3), n_theta=512, n_phi=512)
    hg_sh.astype(np.float32).tofile("hg_sh.bin")
    print("HG function components computed.")
    print(hg_sh)

    dir_sh = project_sh_full(directional_light(2 * np.pi / 4, np.pi, sharpness=100), n_theta=512, n_phi=512)
    dir_sh.astype(np.float32).tofile("dir_sh.bin")
    print("Dir function components computed.")
    print(dir_sh)