import matplotlib.pyplot as plt

N = [100, 200, 300, 400, 500]
time = [0.0366897, 0.310306, 1.0952, 2.42688, 4.87279]

plt.plot(N, time, 'bo-', linewidth=2, markersize=8)
plt.xlabel("Размер матрицы N")
plt.ylabel("Время выполнения (с)")
plt.title("Зависимость времени выполнения от размера матрицы")
plt.grid(True)
plt.savefig("graph.png")
plt.show()