#ifndef ZLE_H
#define ZLE_H

#include <vector>
#include <string>

struct SymX {
    int coef;
    std::string symbol;
};

std::vector<std::vector<int>> szleig(const std::vector<std::vector<SymX>>& A, 
                                     const std::vector<std::string>& symbols, 
                                     int batch_size = -1, 
                                     int stagger = 0, 
                                     int base = 10);

std::vector<std::string> get_symbols(int n);

SymX symx(int i, std::string s = "");

#endif // ZLE_H