import matplotlib.pyplot as plt
import numpy as np

plt.style.use('seaborn-v0_8-darkgrid')

fig, axes = plt.subplots(1, 2, figsize=(14, 5))

sizes = [200, 400, 800, 1200, 1600, 2000]
cpu_times = [0.2165, 1.7656, 10.2694, 36.1968, 88.5648, 315.82]

# Лучшие GPU времена
gpu_times = [0.17773, 0.176111, 0.2346, 0.290668, 0.354897, 0.477212]

# График 1: Время выполнения
ax1 = axes[0]
ax1.plot(sizes, cpu_times, 's--', label='CPU (AMD Ryzen 5 4600H)', linewidth=2, color='red')
ax1.plot(sizes, gpu_times, 'o-', label='GPU (NVIDIA Tesla T4)', linewidth=2, color='green')
ax1.set_xlabel('Matrix Size N', fontsize=12)
ax1.set_ylabel('Time (seconds)', fontsize=12)
ax1.set_title('Execution Time: CPU vs GPU', fontsize=14)
ax1.set_yscale('log')
ax1.legend()
ax1.grid(True, alpha=0.3)

# График 2: Ускорение
ax2 = axes[1]
speedups = [cpu/gpu for cpu, gpu in zip(cpu_times, gpu_times)]
ax2.plot(sizes, speedups, 'o-', linewidth=2, color='purple', markersize=8)
ax2.axhline(y=1, color='gray', linestyle='--', alpha=0.5)
ax2.set_xlabel('Matrix Size N', fontsize=12)
ax2.set_ylabel('Speedup (CPU/GPU)', fontsize=12)
ax2.set_title('GPU Speedup vs CPU', fontsize=14)
ax2.grid(True, alpha=0.3)

# Подписи точек
for size, sp in zip(sizes, speedups):
    ax2.annotate(f'{sp:.1f}x', (size, sp), textcoords="offset points", xytext=(0,10), ha='center')

plt.tight_layout()
plt.savefig('cuda_speedup.png', dpi=150)
plt.show()

print(f"\nMaximum speedup: {max(speedups):.1f}x at N={sizes[speedups.index(max(speedups))]}")