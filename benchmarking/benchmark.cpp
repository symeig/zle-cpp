#include <vector>
#include <iterator>
#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <zle.h>

std::vector<std::vector<int>> readMatrixFromCSV(const std::string& filename) {
    std::vector<std::vector<int>> matrix;
    std::ifstream file(filename);
    std::string line;

    while (std::getline(file, line)) {
        std::vector<int> row;
        std::stringstream ss(line);
        std::string cell;

        while (std::getline(ss, cell, ',')) {
            row.push_back(std::stoi(cell));
        }

        matrix.push_back(row);
    }

    return matrix;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <csv_file>" << std::endl;
        return 1;
    }

    std::string csvFilename = argv[1];
    std::vector<std::vector<int>> indexMatrix = readMatrixFromCSV(csvFilename);

    int n = indexMatrix.size();
    std::vector<std::string> s = get_symbols(n);

    std::vector<std::vector<SymX>> A(n, std::vector<SymX>(n));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            A[i][j] = symx(1, s[indexMatrix[i][j]]);
        }
    }

    int batch_size = n;
    int stagger = 0;
    int base = 10;

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::vector<int>> result = zle_eigs(A, s, batch_size, stagger, base);

    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Finished in " << duration.count() << " milliseconds" << std::endl;

    // Print results
    std::cout << "Consolidated final result:" << std::endl;
    std::cout << "[";
    for (int i = 0; i < n; ++i) {
        std::cout << "[";
        for (int j = 0; j < n; ++j) {
            int coeff = result[i][j];
            std::cout << coeff << ",";
        }
        std::cout << "]," << std::endl;
    }
    std::cout << "]" << std::endl;

    return 0;
}