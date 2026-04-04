import matplotlib.pyplot as plt
import numpy as np

# Данные из экспериментов
sizes = [200, 400, 800, 1200, 1600, 2000]
processes = [1, 2, 4, 8]

# Время выполнения (секунды) - из ваших замеров
time_data = {
    200: [0.0940, 0.0502, 0.0257, 0.0258],
    400: [0.7595, 0.3869, 0.2108, 0.1599],
    800: [6.3057, 3.1650, 1.7144, 1.3789],
    1200: [27.6960, 14.2888, 7.3611, 5.3460],
    1600: [69.1749, 34.5284, 18.0623, 13.8054],
    2000: [199.8800, 113.9010, 74.9647, 50.0221]
}

# Настройка стиля (как во 2 лабе)
plt.style.use('seaborn-v0_8-darkgrid')
colors = plt.cm.viridis(np.linspace(0, 1, len(sizes)))

# =============================================
# ГРАФИК 1: Зависимость времени от количества процессов
# =============================================
plt.figure(figsize=(10, 7))

for i, size in enumerate(sizes):
    plt.plot(processes, time_data[size], 'o-', color=colors[i],
             linewidth=2, markersize=8, label=f'N={size}')

plt.xlabel('Количество процессов', fontsize=14)
plt.ylabel('Время выполнения (с)', fontsize=14)
plt.title('MPI: Зависимость времени выполнения от количества процессов', fontsize=16)
plt.legend(loc='upper right', fontsize=10)
plt.grid(True, alpha=0.3)
plt.xticks(processes)

plt.tight_layout()
plt.savefig('mpi_time_vs_processes.png', dpi=150, bbox_inches='tight')
plt.show()

# =============================================
# ГРАФИК 2: Ускорение (Speedup)
# =============================================
plt.figure(figsize=(10, 7))

for i, size in enumerate(sizes):
    t1 = time_data[size][0]
    speedup = [t1 / t for t in time_data[size]]
    plt.plot(processes, speedup, 'o-', color=colors[i],
             linewidth=2, markersize=8, label=f'N={size}')

# Идеальное ускорение
plt.plot(processes, processes, 'k--', alpha=0.5, linewidth=2, label='Идеальное ускорение')

plt.xlabel('Количество процессов', fontsize=14)
plt.ylabel('Ускорение (Speedup)', fontsize=14)
plt.title('MPI: Ускорение при увеличении количества процессов', fontsize=16)
plt.legend(loc='upper left', fontsize=10)
plt.grid(True, alpha=0.3)
plt.xticks(processes)

plt.tight_layout()
plt.savefig('mpi_speedup_vs_processes.png', dpi=150, bbox_inches='tight')
plt.show()


# =============================================
# Вывод статистики
# =============================================
print("\n" + "=" * 60)
print("СТАТИСТИКА РЕЗУЛЬТАТОВ MPI")
print("=" * 60)

for size in sizes:
    times = time_data[size]
    best_time = min(times)
    best_proc = processes[np.argmin(times)]
    t1 = times[0]
    t8 = times[3] if len(times) > 3 else times[-1]
    speedup_8 = t1 / t8 if t8 > 0 else 1

    print(f"\nN={size}:")
    print(f"  Лучшее время: {best_time:.4f} с на {best_proc} процессах")
    print(f"  Ускорение на 8 процессах: {speedup_8:.2f}x")
    print(f"  Эффективность на 8 процессах: {(speedup_8 / 8) * 100:.1f}%")