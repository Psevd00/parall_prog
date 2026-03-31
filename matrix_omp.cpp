#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>
#include <omp.h>

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

vector<vector<double>> multiply(vector<vector<double>> A, vector<vector<double>> B, int num_threads) {
    int n = A.size();
    vector<vector<double>> C(n, vector<double>(n, 0));

    // Устанавливаем количество потоков
    omp_set_num_threads(num_threads);

    // Параллельное умножение матриц
    #pragma omp parallel for collapse(2) schedule(static)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0.0;
            for (int k = 0; k < n; k++) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }

    return C;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cout << "Usage: " << argv[0] << " <matrix1_file> <matrix2_file> <num_threads>" << endl;
        cout << "Example: " << argv[0] << " matrix_1.txt matrix_2.txt 4" << endl;
        return 1;
    }

    string fileA = argv[1];
    string fileB = argv[2];
    int num_threads = atoi(argv[3]);

    int n1, n2;

    vector<vector<double>> A = readMatrix(fileA, n1);
    vector<vector<double>> B = readMatrix(fileB, n2);

    if (n1 != n2) {
        cout << "Matrices must be same size!" << endl;
        return 1;
    }

    int n = n1;
    
    cout << "Matrix size: " << n << "x" << n << endl;
    cout << "Threads: " << num_threads << endl;

    auto start = chrono::high_resolution_clock::now();

    vector<vector<double>> C = multiply(A, B, num_threads);

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;

    writeMatrix("result.txt", C);

    long long operations = 2LL * n * n * n;

    cout << "Execution time: " << duration.count() << " seconds" << endl;
    cout << "Operations: " << operations << endl;
    
    // Вычисляем производительность в GFLOPS
    double gflops = operations / (duration.count() * 1e9);
    cout << "Performance: " << gflops << " GFLOPS" << endl;

    return 0;
}