#include <vector>
#include <iterator>
#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <numeric>
#include <cmath>
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

long long runBenchmark(const std::vector<std::vector<SymX>>& A, const std::vector<std::string>& s, int batch_size, int stagger, int base) {
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::vector<int>> result = zle_eigs(A, s, batch_size, stagger, base);

    auto end = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
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

    const int num_runs = 3;
    std::vector<long long> durations;

    std::cout << "Running " << num_runs << " iterations..." << std::endl;

    for (int i = 0; i < num_runs; ++i) {
        long long duration = runBenchmark(A, s, batch_size, stagger, base);
        durations.push_back(duration);
        std::cout << "Run " << i + 1 << " finished in " << duration << " milliseconds" << std::endl;
    }

    // Calculate average
    long long total_duration = std::accumulate(durations.begin(), durations.end(), 0LL);
    double average_duration = static_cast<double>(total_duration) / num_runs;

    // Calculate variance
    double variance = 0.0;
    for (const auto& duration : durations) {
        variance += std::pow(duration - average_duration, 2);
    }
    variance /= num_runs;

    // Calculate standard deviation
    double std_deviation = std::sqrt(variance);

    std::cout << "Matrix size: " << n << "x" << n << std::endl;
    std::cout << "Average time over " << num_runs << " runs: " << average_duration << " milliseconds" << std::endl;
    std::cout << "Variance: " << variance << " milliseconds^2" << std::endl;
    std::cout << "Standard Deviation: " << std_deviation << " milliseconds" << std::endl;

    // Log results to a CSV file
    std::ofstream logFile("benchmark_results.csv", std::ios_base::app); // Open in append mode
    if (logFile.is_open()) {
        logFile << n << "," << average_duration << "," << variance << "," << std_deviation << std::endl;
        logFile.close();
    } else {
        std::cerr << "Unable to open log file for writing." << std::endl;
    }

    return 0;
}