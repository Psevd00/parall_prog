# Лабораторная работа №1

**Студент:** Астафьев Вадим Алексеевич  
**Группа:** 6311-100503D

## 1 Цель работы

Изучить методы обработки файлов на языке C++, реализовать алгоритм перемножения квадратных матриц, провести исследование зависимости времени выполнения программы от размера задачи.

## 2 Теоретические сведения

Произведение матриц определяется формулой:

$$C_{ij} = \sum_{k=1}^{n} A_{ik} \cdot B_{kj}$$

где:
- A — первая матрица
- B — вторая матрица
- C — результирующая матрица

Количество операций при умножении квадратных матриц размерности N:

$$Operations = 2N^3$$

Алгоритмическая сложность:

$$O(N^3)$$

## 3 Описание алгоритма

Алгоритм работы программы:

1. Считать матрицу А из файла `matrix_1.txt`
2. Считать матрицу В из файла `matrix_2.txt`
3. Проверить совпадение размерностей
4. Выполнить перемножение матриц (тройной вложенный цикл)
5. Сохранить результат в файл `result.txt`
6. Измерить время выполнения
7. Вывести объём вычислений

## 4 Исходный код программы

```cpp
#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>

using namespace std;

vector<vector<double>> readMatrix(string filename, int& n) {
    ifstream file(filename);
    file >> n;

    vector<vector<double>> matrix(n, vector<double>(n));

    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            file >> matrix[i][j];

    file.close();
    return matrix;
}

void writeMatrix(string filename, vector<vector<double>> matrix) {
    ofstream file(filename);
    int n = matrix.size();

    file << n << endl;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
            file << matrix[i][j] << " ";
        file << endl;
    }

    file.close();
}

vector<vector<double>> multiply(vector<vector<double>> A, vector<vector<double>> B) {
    int n = A.size();

    vector<vector<double>> C(n, vector<double>(n, 0));

    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            for (int k = 0; k < n; k++)
                C[i][j] += A[i][k] * B[k][j];

    return C;
}

int main() {
    int n1, n2;

    vector<vector<double>> A = readMatrix("matrix_1.txt", n1);
    vector<vector<double>> B = readMatrix("matrix_2.txt", n2);

    if (n1 != n2) {
        cout << "Matrices must be same size or check your file's name!" << endl;
        return 1;
    }

    auto start = chrono::high_resolution_clock::now();

    vector<vector<double>> C = multiply(A, B);

    auto end = chrono::high_resolution_clock::now();

    chrono::duration<double> duration = end - start;

    writeMatrix("result.txt", C);

    long long operations = 2LL * n1 * n1 * n1;

    cout << "Matrix size: " << n1 << "x" << n1 << endl;
    cout << "Execution time: " << duration.count() << " seconds" << endl;
    cout << "Operations (task volume): " << operations << endl;

    return 0;
}
```

## 5 Формат входных данных

matrix_1.txt:
```
3
1 1 1
2 2 2
3 3 3
```

matrix_2.txt:
```
3
4 4 4
5 5 5
6 6 6
```

## 6 Формат выходных данных

```
3
15 15 15 
30 30 30 
45 45 45 
```

## 7 Автоматическая верификация результатов

Для проверки корректности используется Python и библиотека NumPy.

```python
import numpy as np

def read_matrix(filename):
    with open(filename) as f:
        n = int(f.readline())
        data = []
        for _ in range(n):
            data.append(list(map(float, f.readline().split())))
        return np.array(data)

A = read_matrix("matrixA.txt")
B = read_matrix("matrixB.txt")
C = read_matrix("result.txt")

C_expected = np.dot(A, B)

if np.allclose(C, C_expected, atol=1e-10):
    print("VERIFICATION SUCCESS: Results match!")
    print(f"Max difference: {np.max(np.abs(C - C_expected)):.2e}")
else:
    print("VERIFICATION FAILED: Results do NOT match!")
    print(f"Max difference: {np.max(np.abs(C - C_expected)):.2e}")
```

**Результат выполнения:**
```
VERIFICATION SUCCESS: Results match!
Max difference: 0.00e+00
```

## 8 Исследование программы

Для исследования зависимости времени выполнения от размера задачи были проведены эксперименты с различными размерами матриц.

| Размер матрицы N  | Кол-во операций | Время выполнения(с) |
| --- | :---: | ---: |
| 100 | 2·10⁶ | 0.0366897 |
| 200 | 1.6·10⁷ | 0.310306 |
| 300 | 5.4·10⁷ | 1.0952 |
| 400 | 1.28·10⁸ | 2.42688 |
| 500 | 2.5·10⁸ | 4.87279 |


## 9 График зависимости времени

[![График зависимости времени]](https://github.com/Psevd00/parall_prog/blob/main/Lab_1/graph.png)

## 10 Анализ результатов

Из полученных данных видно, что:

При увеличении размера матрицы в 2 раза (с 100 до 200) время выполнения увеличивается примерно в [N] раз

При увеличении размера в 5 раз (с 100 до 500) время увеличивается примерно в 125 раз, что соответствует теоретической сложности O(N³)

## 11 Вывод

В ходе выполнения лабораторной работы была разработана программа для перемножения квадратных матриц на языке C++. Программа корректно читает входные данные из файлов, выполняет умножение и сохраняет результат.

Проведена автоматическая верификация с использованием Python и библиотеки NumPy, которая подтвердила правильность вычислений (максимальное отклонение 0).

Экспериментально исследована зависимость времени выполнения от размера матрицы. Результаты подтверждают теоретическую сложность алгоритма O(N³) — время выполнения растёт пропорционально кубу размера матрицы.
