# Лабораторная работа №5
## Параллельное перемножение матриц на суперкомпьютере

**Студент:** Астафьев Вадим Алексеевич
**Группа:** 6311-100503D

---

## 1 Цель работы

Изучить основы параллельного программирования с использованием MPI, реализовать параллельный алгоритм перемножения квадратных матриц, исследовать масштабируемость программы на суперкомпьютере «Сергей Королёв» и оценить ускорение при увеличении числа процессов.

## 2 Теоретические сведения

Произведение матриц определяется формулой:

$$C_{ij} = \sum_{k=1}^{n} A_{ik} \cdot B_{kj}$$

где:
- A — первая матрица,
- B — вторая матрица,
- C — результирующая матрица.

Количество операций при умножении квадратных матриц размерности N:

$$Operations = 2N^3$$

Алгоритмическая сложность:

$$O(N^3)$$

В параллельной реализации используется **одномерная декомпозиция** по строкам: каждая MPI-процесс получает свою часть строк матрицы A и полную копию матрицы B (после транспонирования для улучшения локальности). Результат собирается на главном процессе.

## 3 Описание параллельного алгоритма

Алгоритм работы MPI-программы:

1. Процесс с рангом 0 считывает матрицы A и B из файлов `matrix_1.txt` и `matrix_2.txt`.
2. Размер матрицы N рассылается всем процессам с помощью `MPI_Bcast`.
3. Матрица A распределяется между процессами построчно с использованием `MPI_Scatterv` (учитывается возможный неравномерный остаток строк).
4. Матрица B копируется во все процессы через `MPI_Bcast` и транспонируется для эффективного доступа по столбцам.
5. Каждый процесс выполняет локальное умножение своей части строк A на всю матрицу B.
6. Результаты собираются на процессе 0 с помощью `MPI_Gatherv`.
7. Процесс 0 записывает результирующую матрицу в файл `result.txt` и выводит время выполнения.

## 4 Исходный код программы (MPI)

```cpp
#include "mpi.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>

using namespace std;

int ProcNum, ProcRank;

void Flip(double *B, int Size)
{
    double temp = 0.0;
    for (int i = 0; i < Size; i++)
        for (int j = i + 1; j < Size; j++)
        {
            temp = B[i * Size + j];
            B[i * Size + j] = B[j * Size + i];
            B[j * Size + i] = temp;
        }
}

void MatrixMultiplicationMPI(double *A, double *B, double *C, int Size)
{
    int dim = Size;
    int ProcPartSize = dim / ProcNum;
    int remainder = dim % ProcNum;

    int *sendcounts = new int[ProcNum];
    int *displs = new int[ProcNum];
    int offset = 0;
    for (int i = 0; i < ProcNum; i++)
    {
        int rows = (i < remainder) ? (ProcPartSize + 1) : ProcPartSize;
        sendcounts[i] = rows * dim;
        displs[i] = offset;
        offset += sendcounts[i];
    }

    int local_rows = sendcounts[ProcRank] / dim;
    double *local_A = new double[sendcounts[ProcRank]];
    double *local_C = new double[sendcounts[ProcRank]];

    MPI_Scatterv(A, sendcounts, displs, MPI_DOUBLE,
                 local_A, sendcounts[ProcRank], MPI_DOUBLE,
                 0, MPI_COMM_WORLD);

    double *B_copy = new double[dim * dim];
    MPI_Bcast(B_copy, dim * dim, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    Flip(B_copy, dim);

    for (int i = 0; i < local_rows; i++)
        for (int j = 0; j < dim; j++)
        {
            double sum = 0.0;
            for (int k = 0; k < dim; k++)
                sum += local_A[i * dim + k] * B_copy[j * dim + k];
            local_C[i * dim + j] = sum;
        }

    int *recvcounts = new int[ProcNum];
    int *recvdispls = new int[ProcNum];
    for (int i = 0; i < ProcNum; i++)
    {
        recvcounts[i] = sendcounts[i];
        recvdispls[i] = displs[i];
    }
    MPI_Gatherv(local_C, sendcounts[ProcRank], MPI_DOUBLE,
                C, recvcounts, recvdispls, MPI_DOUBLE,
                0, MPI_COMM_WORLD);

    delete[] local_A;
    delete[] local_C;
    delete[] B_copy;
    delete[] sendcounts;
    delete[] displs;
    delete[] recvcounts;
    delete[] recvdispls;
}

double* readMatrix(const char* filename, int &n)
{
    ifstream file(filename);
    file >> n;
    double *mat = new double[n * n];
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            file >> mat[i * n + j];
    file.close();
    return mat;
}

void writeMatrix(const char* filename, double *mat, int n)
{
    ofstream file(filename);
    file << n << endl;
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
            file << mat[i * n + j] << " ";
        file << endl;
    }
    file.close();
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);

    int n1, n2;
    double *A = NULL, *B = NULL, *C = NULL;

    if (ProcRank == 0)
    {
        A = readMatrix("matrix_1.txt", n1);
        B = readMatrix("matrix_2.txt", n2);
        if (n1 != n2)
        {
            cerr << "Matrix sizes do not match!" << endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    MPI_Bcast(&n1, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (ProcRank == 0)
        C = new double[n1 * n1];

    double start_time = MPI_Wtime();
    MatrixMultiplicationMPI(A, B, C, n1);
    double end_time = MPI_Wtime();

    if (ProcRank == 0)
    {
        cout << "Matrix size: " << n1 << "x" << n1 << endl;
        cout << "Number of processes: " << ProcNum << endl;
        cout << "Execution time: " << (end_time - start_time) << " seconds" << endl;
        long long operations = 2LL * n1 * n1 * n1;
        cout << "Operations (task volume): " << operations << endl;
        writeMatrix("result.txt", C, n1);
    }

    if (ProcRank == 0)
    {
        delete[] A;
        delete[] B;
        delete[] C;
    }

    MPI_Finalize();
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
### 9 Формат вывода файлов в формате .out


slurm-251652.out:
```
Matrix size: 200x200
Number of processes: 1
Execution time: 0.0462329 seconds
Operations (task volume): 16000000

```



### 8 Результаты эксперемента

| Размер N | 1 процесс | 2 процесса | 4 процесса | 8 процессов |
|----------|-----------|------------|------------|-------------|
| 200      | 0.04623    | 0.02397    | 0.01249    | 0.00679     |
| 400      | 0.35730    | 0.18154    | 0.09410    | 0.04913     |
| 800      | 2.74149    | 1.42719    | 0.73052    | 0.38172     |
| 1200     | 8.69042    | 4.48116    | 2.38304    | 1.26705     |
| 1600     | 20.7768    | 10.5119    | 5.37872    | 2.82211     |
| 2000     | 40.4116    | 20.3198    | 10.4127    | 5.5921      |

---


## 9 Графики

[Нрафик сравнения времени выполнения относительно кол-ва процессоров](https://github.com/Psevd00/parall_prog/blob/lab5/graph_time.png)
[График ускорения](https://github.com/Psevd00/parall_prog/blob/lab5/graph_speedup.png)

## 10 Анализ результатов

- **Масштабируемость**: с ростом числа процессов время выполнения уменьшается практически обратно пропорционально числу процессов. Для матрицы 2000×2000 ускорение на 4 процессах достигает 3.88 (эффективность 97%).
- **Влияние размера задачи**: для малых матриц (200×200) накладные расходы на коммуникацию заметнее – эффективность на 8 процессах составляет 85%. С увеличением размера задачи эффективность растёт и для 1600×1600 достигает 92% на 8 процессах.
- **Соответствие теории**: время выполнения последовательной версии (1 процесс) растёт как ~2N³, что подтверждает O(N³). Параллельная версия демонстрирует хорошую масштабируемость, ограниченную только коммуникационными затратами и неравномерностью распределения строк (при N, не кратном числу процессов).

## 11 Вывод

В ходе выполнения лабораторной работы была разработана параллельная программа умножения квадратных матриц с использованием MPI. Программа корректно работает на суперкомпьютере «Сергей Королёв», поддерживает произвольное число процессов и автоматически распределяет строки матрицы A.

Проведённые эксперименты показали:

- При увеличении числа процессов время выполнения сокращается почти пропорционально их количеству для задач большого размера.
- Эффективность параллелизации для матрицы 1600×1600 на 8 процессах составляет 92%, что говорит о низких накладных расходах на коммуникацию.
- Полученные результаты могут быть использованы для дальнейшего изучения более сложных схем параллельного умножения (например, блочных или с использованием MPI-топологий).

Таким образом, цель работы достигнута: освоены базовые принципы MPI, реализован эффективный параллельный алгоритм и проведён анализ его производительности.


