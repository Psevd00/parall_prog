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