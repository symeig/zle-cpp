#include <vector>
#include <iterator>
#include <chrono>
#include <iostream>

#include <zle.h>

int main() {
    // Example usage
    int n = 377;
    std::vector<std::string> s = get_symbols(n);

    int batch_size = 48;
    int stagger = 0;
    int base = 10;

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::vector<int>> result = zle_eigs(A, s, batch_size, stagger, base);

    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "finished in " << duration.count() << " milliseconds" << std::endl;

    // Print results
    std::cout << "consolidated final result:" << std::endl;
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