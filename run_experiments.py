import subprocess
import csv
import time

# Параметры экспериментов
sizes = [200, 400, 800, 1200, 1600, 2000]
threads = [1, 2, 4, 8]

# Создаём CSV файл для результатов
with open('results.csv', 'w', newline='') as f:
    writer = csv.writer(f)
    writer.writerow(['Size', 'Threads', 'Time', 'Speedup', 'Efficiency'])

# Для каждого размера и количества потоков
for size in sizes:
    print(f"\n=== Testing size {size} ===")

    # Создаём тестовые матрицы
    print(f"Generating matrices...")
    # Здесь можно добавить генерацию матриц

    # Замер для последовательного выполнения (1 поток) - baseline
    print(f"  Running with 1 thread...")
    result = subprocess.run(['./matrix_omp.exe', f'matrix_{size}_1.txt', f'matrix_{size}_2.txt', '1'],
                            capture_output=True, text=True)

    # Парсим время из вывода
    lines = result.stdout.split('\n')
    time_1 = 0
    for line in lines:
        if 'Execution time' in line:
            time_1 = float(line.split(':')[1].strip().split()[0])

    # Замер для разного количества потоков
    for t in threads:
        print(f"  Running with {t} threads...")
        result = subprocess.run(['./matrix_omp.exe', f'matrix_{size}_1.txt', f'matrix_{size}_2.txt', str(t)],
                                capture_output=True, text=True)

        # Парсим результат
        lines = result.stdout.split('\n')
        time_t = 0
        for line in lines:
            if 'Execution time' in line:
                time_t = float(line.split(':')[1].strip().split()[0])

        # Вычисляем ускорение и эффективность
        speedup = time_1 / time_t
        efficiency = speedup / t

        # Сохраняем
        with open('results.csv', 'a', newline='') as f:
            writer = csv.writer(f)
            writer.writerow([size, t, round(time_t, 4), round(speedup, 3), round(efficiency, 3)])

        print(f"    Time: {time_t:.4f}s, Speedup: {speedup:.2f}, Efficiency: {efficiency:.2f}")

print("\nExperiments completed! Results saved to results.csv")