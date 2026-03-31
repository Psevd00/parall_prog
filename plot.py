import matplotlib.pyplot as plt
import numpy as np

# Данные из вашей таблицы
sizes = [200, 400, 600, 800, 1200, 1600, 2000]
threads = [1, 2, 4, 6, 8, 10, 12]

# Время выполнения (секунды)
time_data = {
    200:  [0.216486, 0.1287, 0.0845, 0.0716, 0.0636, 0.0785, 0.0806],
    400:  [1.7656, 1.0534, 0.6306, 0.4983, 0.4584, 0.3979, 0.3443],
    600:  [6.0087, 3.4710, 1.9657, 1.4721, 1.1179, 1.0246, 1.0302],
    800:  [10.2694, 6.19587, 4.19683, 3.00783, 2.6346, 2.31059, 2.21248],
    1200: [36.1968, 21.0210, 10.9910, 7.9068, 6.6466, 5.9543, 5.7047],
    1600: [88.5648, 47.6581, 27.3235, 20.3801, 16.9392, 15.2751, 14.5075],
    2000: [315.82, 288.357, 199.789, 130.310, 106.272, 92.129, 87.942]
}

# Настройка стиля
plt.style.use('seaborn-v0_8-darkgrid')

# Цвета для разных размеров
colors = plt.cm.viridis(np.linspace(0, 1, len(sizes)))

# =============================================
# ГРАФИК 1: Зависимость времени от потоков
# =============================================
plt.figure(figsize=(10, 7))

for i, size in enumerate(sizes):
    plt.plot(threads, time_data[size], 'o-', color=colors[i], linewidth=2, markersize=6, label=f'N={size}')

plt.xlabel('Количество потоков', fontsize=14)
plt.ylabel('Время выполнения (с)', fontsize=14)
plt.title('Зависимость времени выполнения от количества потоков', fontsize=16)
plt.yscale('log')
plt.legend(loc='upper right', fontsize=10, ncol=2)
plt.grid(True, alpha=0.3)
plt.xticks(threads)

plt.tight_layout()
plt.savefig('time_vs_threads.png', dpi=150, bbox_inches='tight')
plt.show()

# =============================================
# ГРАФИК 2: Ускорение (Speedup)
# =============================================
plt.figure(figsize=(10, 7))

for i, size in enumerate(sizes):
    t1 = time_data[size][0]
    speedup = [t1 / t for t in time_data[size]]
    plt.plot(threads, speedup, 'o-', color=colors[i], linewidth=2, markersize=6, label=f'N={size}')

# Идеальное ускорение
plt.plot([1, 12], [1, 12], 'k--', alpha=0.5, linewidth=2, label='Идеальное ускорение')

plt.xlabel('Количество потоков', fontsize=14)
plt.ylabel('Ускорение (Speedup)', fontsize=14)
plt.title('Ускорение при увеличении количества потоков', fontsize=16)
plt.legend(loc='upper left', fontsize=10, ncol=2)
plt.grid(True, alpha=0.3)
plt.xticks(threads)

plt.tight_layout()
plt.savefig('speedup_vs_threads.png', dpi=150, bbox_inches='tight')
plt.show()

# =============================================
# Вывод статистики
# =============================================
print("\n" + "="*60)
print("СТАТИСТИКА РЕЗУЛЬТАТОВ")
print("="*60)

for size in sizes:
    t1 = time_data[size][0]
    t12 = time_data[size][6]
    speedup_12 = t1 / t12
    best_time = min(time_data[size])
    best_threads = threads[np.argmin(time_data[size])]
    print(f"\nN={size}:")
    print(f"  Лучшее время: {best_time:.4f} с на {best_threads} потоках")
    print(f"  Ускорение на 12 потоках: {speedup_12:.2f}x")
    print(f"  Эффективность на 12 потоках: {(speedup_12/12)*100:.1f}%")