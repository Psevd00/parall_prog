#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <mpi.h>

using namespace std;

// Чтение матрицы из файла (только на главном процессе)
vector<vector<double>> readMatrix(const string& filename, int& n) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Cannot open file " << filename << endl;
        exit(1);
    }
    
    file >> n;
    vector<vector<double>> matrix(n, vector<double>(n));
    
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            file >> matrix[i][j];
    
    file.close();
    return matrix;
}

// Запись матрицы в файл (только на главном процессе)
void writeMatrix(const string& filename, const vector<vector<double>>& matrix) {
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

int main(int argc, char* argv[]) {
    // Инициализация MPI
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    if (argc != 3) {
        if (rank == 0) {
            cout << "Usage: mpiexec -np N ./matrix_mpi.exe <matrix1_file> <matrix2_file>" << endl;
        }
        MPI_Finalize();
        return 1;
    }
    
    string fileA = argv[1];
    string fileB = argv[2];
    
    int n = 0;
    vector<vector<double>> A, B;
    vector<double> flatA;
    
    double start_time = 0.0;
    
    // ========== Чтение данных (только на процессе 0) ==========
    if (rank == 0) {
        cout << "Reading matrices..." << endl;
        A = readMatrix(fileA, n);
        B = readMatrix(fileB, n);
        
        // Проверка размерностей
        if (A.size() != n || B.size() != n) {
            cerr << "Error: Matrix dimensions mismatch!" << endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        
        cout << "Matrix size: " << n << "x" << n << endl;
        cout << "Number of processes: " << size << endl;
        
        // Преобразуем A в плоский массив для рассылки
        flatA.resize(n * n);
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                flatA[i * n + j] = A[i][j];
        
        start_time = MPI_Wtime();
    }
    
    // ========== Рассылка размера матрицы ==========
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    if (n == 0) {
        MPI_Finalize();
        return 1;
    }
    
    // ========== Подготовка матрицы B (полная копия всем процессам) ==========
    vector<double> flatB(n * n);
    if (rank == 0) {
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                flatB[i * n + j] = B[i][j];
    }
    
    // Рассылаем B всем процессам
    MPI_Bcast(flatB.data(), n * n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    
    // Восстанавливаем B из плоского массива
    vector<vector<double>> B_local(n, vector<double>(n));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            B_local[i][j] = flatB[i * n + j];
    
    // ========== Распределение строк матрицы A ==========
    // Определяем, сколько строк обработает каждый процесс
    int rows_per_proc = n / size;
    int remainder = n % size;
    
    vector<int> sendcounts(size), displs(size);
    int offset = 0;
    for (int i = 0; i < size; i++) {
        sendcounts[i] = (i < remainder) ? (rows_per_proc + 1) : rows_per_proc;
        displs[i] = offset;
        offset += sendcounts[i];
    }
    
    // Локальный буфер для строк A
    int local_rows = sendcounts[rank];
    vector<double> localA(local_rows * n);
    
    // Распределяем строки A между процессами
    MPI_Scatterv(flatA.data(), sendcounts.data(), displs.data(), MPI_DOUBLE,
                 localA.data(), local_rows * n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    
    // ========== Вычисление ==========
    vector<double> localC(local_rows * n, 0.0);
    
    for (int i = 0; i < local_rows; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0.0;
            for (int k = 0; k < n; k++) {
                sum += localA[i * n + k] * B_local[k][j];
            }
            localC[i * n + j] = sum;
        }
    }
    
    // ========== Сбор результатов ==========
    vector<double> flatC;
    if (rank == 0) {
        flatC.resize(n * n);
    }
    
    // Для MPI_Gatherv нужно передать количество элементов от каждого процесса
    vector<int> recvcounts(size);
    for (int i = 0; i < size; i++) {
        recvcounts[i] = sendcounts[i] * n;  // количество элементов, а не строк
    }
    
    MPI_Gatherv(localC.data(), local_rows * n, MPI_DOUBLE,
                flatC.data(), recvcounts.data(), displs.data(), MPI_DOUBLE,
                0, MPI_COMM_WORLD);
    
    double end_time = 0.0;
    if (rank == 0) {
        end_time = MPI_Wtime();
        double elapsed = end_time - start_time;
        
        // Восстанавливаем матрицу C
        vector<vector<double>> C(n, vector<double>(n));
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                C[i][j] = flatC[i * n + j];
        
        // Сохраняем результат
        writeMatrix("result_mpi.txt", C);
        
        // Вывод результатов
        long long operations = 2LL * n * n * n;
        double gflops = operations / (elapsed * 1e9);
        
        cout << "Execution time: " << elapsed << " seconds" << endl;
        cout << "Operations: " << operations << endl;
        cout << "Performance: " << gflops << " GFLOPS" << endl;
    }
    
    MPI_Finalize();
    return 0;
}