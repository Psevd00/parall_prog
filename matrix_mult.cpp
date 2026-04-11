#include "mpi.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>

using namespace std;

int ProcNum, ProcRank;

// Транспонирование матрицы (для улучшения доступа к памяти)
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

// Параллельное умножение матриц
void MatrixMultiplicationMPI(double *A, double *B, double *C, int Size)
{
    int dim = Size;
    int ProcPartSize = dim / ProcNum;
    int remainder = dim % ProcNum;

    // Сколько элементов (строк * dim) получит каждый процесс
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

    // Раздаём строки матрицы A
    MPI_Scatterv(A, sendcounts, displs, MPI_DOUBLE,
                 local_A, sendcounts[ProcRank], MPI_DOUBLE,
                 0, MPI_COMM_WORLD);

    // Копируем B во все процессы и транспонируем
    double *B_copy = new double[dim * dim];
    MPI_Bcast(B_copy, dim * dim, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    Flip(B_copy, dim);  // транспонируем для кэш-локальности

    // Локальное умножение
    for (int i = 0; i < local_rows; i++)
        for (int j = 0; j < dim; j++)
        {
            double sum = 0.0;
            for (int k = 0; k < dim; k++)
                sum += local_A[i * dim + k] * B_copy[j * dim + k];
            local_C[i * dim + j] = sum;
        }

    // Собираем результат
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

    // Очистка памяти
    delete[] local_A;
    delete[] local_C;
    delete[] B_copy;
    delete[] sendcounts;
    delete[] displs;
    delete[] recvcounts;
    delete[] recvdispls;
}

// Чтение матрицы из файла (возвращает указатель на массив и размер)
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

// Запись матрицы в файл
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

    // Процесс 0 читает матрицы из файлов
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

    // Рассылаем размер матрицы всем процессам
    MPI_Bcast(&n1, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Выделяем память для результата (только на процессе 0)
    if (ProcRank == 0)
        C = new double[n1 * n1];

    // Замер времени (используем MPI_Wtime, так как clock() не годится для параллельных)
    double start_time = MPI_Wtime();

    // Выполняем умножение
    MatrixMultiplicationMPI(A, B, C, n1);

    double end_time = MPI_Wtime();

    // Вывод времени (только процесс 0)
    if (ProcRank == 0)
    {
        cout << "Matrix size: " << n1 << "x" << n1 << endl;
        cout << "Number of processes: " << ProcNum << endl;
        cout << "Execution time: " << (end_time - start_time) << " seconds" << endl;
        long long operations = 2LL * n1 * n1 * n1;
        cout << "Operations (task volume): " << operations << endl;

        // Запись результата
        writeMatrix("result.txt", C, n1);
    }

    // Освобождение памяти
    if (ProcRank == 0)
    {
        delete[] A;
        delete[] B;
        delete[] C;
    }

    MPI_Finalize();
    return 0;
}