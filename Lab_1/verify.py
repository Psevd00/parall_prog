import numpy as np

def read_matrix(filename):
    with open(filename) as f:
        n = int(f.readline())
        data = []
        for _ in range(n):
            data.append(list(map(float, f.readline().split())))
        return np.array(data)

# Чтение матриц
A = read_matrix("matrix_1.txt")
B = read_matrix("matrix_2.txt")
C = read_matrix("result.txt")

# Эталонное умножение
C_expected = np.dot(A, B)

# Проверка
if np.allclose(C, C_expected, atol=1e-10):
    print("VERIFICATION SUCCESS: Results match!")
    print(f"Max difference: {np.max(np.abs(C - C_expected)):.2e}")
else:
    print("VERIFICATION FAILED: Results do NOT match!")
    print(f"Max difference: {np.max(np.abs(C - C_expected)):.2e}")