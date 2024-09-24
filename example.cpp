#include <vector>
#include <iterator>
#include <chrono>
#include <iostream>

#include <zle.h>

int main() {
    // Example usage
    int n = 8;
    std::vector<std::string> s = get_symbols(n);
    std::vector<std::vector<SymX>> A = {
        {symx(1, s[0]), symx(1, s[1]), symx(1, s[2]), symx(1, s[3]), symx(1, s[4]), symx(1, s[5]), symx(1, s[6]), symx(1, s[7])},
        {symx(1, s[1]), symx(1, s[0]), symx(1, s[2]), symx(1, s[4]), symx(1, s[3]), symx(1, s[6]), symx(1, s[5]), symx(1, s[7])},
        {symx(1, s[2]), symx(1, s[1]), symx(1, s[0]), symx(1, s[3]), symx(1, s[4]), symx(1, s[7]), symx(1, s[6]), symx(1, s[5])},
        {symx(1, s[3]), symx(1, s[4]), symx(1, s[2]), symx(1, s[0]), symx(1, s[1]), symx(1, s[5]), symx(1, s[6]), symx(1, s[7])},
        {symx(1, s[4]), symx(1, s[3]), symx(1, s[2]), symx(1, s[1]), symx(1, s[0]), symx(1, s[6]), symx(1, s[5]), symx(1, s[7])},
        {symx(1, s[5]), symx(1, s[6]), symx(1, s[7]), symx(1, s[3]), symx(1, s[4]), symx(1, s[0]), symx(1, s[1]), symx(1, s[2])},
        {symx(1, s[6]), symx(1, s[5]), symx(1, s[7]), symx(1, s[4]), symx(1, s[3]), symx(1, s[1]), symx(1, s[0]), symx(1, s[2])},
        {symx(1, s[7]), symx(1, s[6]), symx(1, s[5]), symx(1, s[3]), symx(1, s[4]), symx(1, s[2]), symx(1, s[1]), symx(1, s[0])}
    };

    int batch_size = 8;
    int stagger = 0;
    int base = 10;

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::vector<int>> result = zle_eigs(A, s, batch_size, stagger, base);

    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "finished in " << duration.count() << " milliseconds" << std::endl;

    // Print results. Each element in the axis dimension is an eigenvalue. 
    // Each element in the second axis is the coefficient on the corresponding 
    // symbol of the same index from s 
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
