#include <iostream>
#include <vector>
#include <complex>
#include <algorithm>
#include <random>
#include <iterator>
#include <cmath>
#include <map>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <chrono>
#include <mplapack/mpblas_mpfr.h>
#include <mplapack/mplapack_mpfr.h>

#include "zle.h"

using mpreal = mpfr::mpreal;
using mpcomplex = mpfr::mpcomplex;

struct ExtractionParams {
    int midpoint_value;
    mpreal midpoint;
};

struct Dynamics {
    std::vector<mpreal> real_values;
    std::vector<mpreal> indicator_values;
};

mpcomplex* psuedo_symbolic_matrix_segment(const std::vector<SymX>& A, int n, int batch_index, int batch_size, const std::vector<std::string>& symbols, const Dynamics& dynamics) {
    std::vector<mpreal> real_values = dynamics.real_values;
    std::vector<mpreal> indicator_values = dynamics.indicator_values;

    mpcomplex *complex_values_for_substitution = new mpcomplex[n];

    // Copy real_values into real part of complex_values_for_substitution
    for (int i = 0; i < n; ++i) {
        complex_values_for_substitution[i] = mpcomplex(real_values[i], 0);
    }

    // Copy indicator_values into imaginary part of complex_values_for_substitution
    for (int i = batch_index * batch_size; i < std::min(n, (batch_index + 1) * batch_size); ++i) {
        complex_values_for_substitution[i] = mpcomplex(real_values[i], indicator_values[i - batch_index * batch_size]);
    }

    // Create mapping from symbols to substitution values
    std::map<std::string, mpcomplex> mapping;
    for (int i = 0; i < n; ++i) {
        mapping[symbols[i]] = complex_values_for_substitution[i];
    }

    // Create a "copy" of A, Af using an mpfr compatible matrix of mpcomplex
    mpcomplex *Af = new mpcomplex[n * n];

    // Elements of Af correspond to elements of A with the symbols replaced by substitution values
    for (int i = 0; i < n * n; ++i) {
        mpreal coef(A[i].coef);
        if (A[i].coef == 0) {
            Af[i] = mpcomplex(coef, coef);
        } else {
            Af[i] = coef * mapping.at(A[i].symbol);
        }    
    }

    delete[] complex_values_for_substitution;
    
    return Af;
}

Dynamics set_dynamics(mpreal base, int n, int digits, int stagger) {
    // (1) create indicator values. set proper bounds on exponents
    int start = static_cast<int>(std::floor(-digits / 2.0));
    int end = static_cast<int>(std::ceil(digits / 2.0)-stagger);
    std::vector<mpreal> indicators;

    for (int i = start; i < end; i+=stagger+1) {
        indicators.push_back(pow(base, i));
    }

    // reverse the vector
    std::reverse(indicators.begin(), indicators.end());
    
    // (2) create n 'random' real values. -needs- to have enough precision here
    std::vector<mpreal> values;
    mpreal inv_exp = -std::floor(digits / 2.0);  // Calculate -Floor[digits / 2]
    for (int i = 1; i <= n; ++i) {
        mpreal m_i = i;  // Convert i to mpreal
        mpreal term = m_i + 1/m_i + pow(m_i, inv_exp);
        values.push_back(term);
    }
    // shuffle the random real values
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(values.begin(), values.end(), g);
    
    Dynamics dynamics;
    dynamics.real_values = values;
    dynamics.indicator_values = indicators;
    return dynamics;
}


ExtractionParams set_extraction_params(const mpreal& base, int digits) {
    ExtractionParams params;
    mpreal midpoint_value = mpfr::floor(base / 2) - 1;
    params.midpoint = 0;

    for (int i = -floor((float)digits / 2); i < ceil((float)digits / 2); i++) {
        params.midpoint += midpoint_value * mpfr::pow(base, i);
    }

    // suspicious chain casting
    params.midpoint_value = (int)(long int)(midpoint_value);
    return params;
}

// Function to convert digits in an arbitrary base to bits
int digits_to_bits(int digits, int base) {
    double log2_base = std::log2(base);
    return static_cast<int>(std::ceil(digits * log2_base));
}

//real-sort method
bool compare_real(const mpcomplex &a, const mpcomplex &b) {
    return a.real() < b.real();
}

//eig
mpcomplex* sorted_eig(mpcomplex* a, mpcomplex* w, int n, int batch_index) {
    mpcomplex *vl = new mpcomplex[n * n];
    mpcomplex *vr = new mpcomplex[n * n];
    mplapackint lwork = 4 * n;
    mpcomplex *work = new mpcomplex[lwork];    
    mpreal *rwork = new mpreal[lwork];
    mplapackint info;
    
    Cgeev("N", "N", n, a, n, w, vl, n, vr, n, work, lwork, rwork, info);
    std::sort(w, w + n, compare_real);
    
    delete[] rwork;
    delete[] work;
    delete[] vr;
    delete[] vl;
    
    //once eigenvalues are computed, complex matrix no longer needed
    delete[] a;
    
    return w;
}

int filler_digits(const mpreal& e, int digits, const mpreal& base) {
    if (mpfr::abs(e) / mpfr::pow(base, mpfr::ceil(digits / 2) - 1) >= 1) {
        return 0;
    }
    int j = 2;
    while (mpfr::abs(e) / mpfr::pow(base, mpfr::ceil(digits / 2) - j) < 1) {
        ++j;
    }

    // suspicious chain casting
    return std::min((int)(long int)(mpfr::ceil(digits / 2) - 1), j - 1);
}

std::string get_strings(const mpreal& ev, int digits, const mpreal& midpoint, int midpoint_value, const mpreal& base) {
    int zero_fill = filler_digits(ev, digits, base);
    int esize = digits - zero_fill;
    std::ostringstream oss;
    oss.precision(esize + 1);
    oss << std::fixed << ev;
    std::string mne_string = oss.str();
    mne_string = std::string(zero_fill, '0') + mne_string;
    return mne_string;
}

std::vector<int> extract_coef_list(const std::string& eigenvalue_string, int midpoint_value) {
    std::vector<int> result;
    for (char c : eigenvalue_string) {
        if (c != '.') {
            int ci = c - '0';
            result.push_back(ci - midpoint_value);
        }
    }
    return result;
}

int ipow(int base, int exp)
{
    int result = 1;
    for (;;)
    {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        if (!exp)
            break;
        base *= base;
    }

    return result;
}

// the result is a list of coefficients for each eigenvalue in an matrix n x batch_size
std::vector<int> extract_symbolic_eigenvalues(const mpcomplex* eigenvalues, int n, int batch_size, int digits, const mpreal& midpoint, int midpoint_value, int stagger, const mpreal& base, int int_base) {
    std::vector<int> flattened_coeffs;
    int coeffs_per_eigenvalue = digits / (stagger + 1);
    
    flattened_coeffs.reserve(n * batch_size); // Pre-allocate memory

    for (int i = 0; i < n; ++i) {
        mpreal midpoint_normalized_eigen = eigenvalues[i].imag() + midpoint;
        std::string mne_string = get_strings(midpoint_normalized_eigen, digits, midpoint, midpoint_value, base);

        std::vector<int> coef_list = extract_coef_list(mne_string, midpoint_value);

        // Stagger computation
        for (size_t j = 0; j < coeffs_per_eigenvalue; ++j) {
            int coeff = 0;
            for (int k = 0; k < stagger + 1 && (j * (stagger + 1) + k) < coef_list.size(); ++k) {
                coeff += ipow(int_base, stagger - k) * coef_list[j * (stagger + 1) + k];
            }
            flattened_coeffs.push_back(coeff);
        }
    }
    
    return flattened_coeffs;
}

// the result is a list of coefficients for each eigenvalue in an implicit matrix n x batch_size
std::vector<int> calculate_eigenvalues_for_batch(const std::vector<SymX>& A, int n, int batch_index, int batch_size, const std::vector<std::string>& symbols, const Dynamics& dynamics, const mpreal& real_base, int int_base, int digits, int stagger, const ExtractionParams& extraction_params) {
    mpcomplex* Ai = psuedo_symbolic_matrix_segment(A, n, batch_index, batch_size, symbols, dynamics);

    std::cout << "calculating eigenvalues for batch " << batch_index << std::endl;

    mpcomplex* psuedo_symbolic_eigenvalues = new mpcomplex[n];

    psuedo_symbolic_eigenvalues = sorted_eig(Ai, psuedo_symbolic_eigenvalues, n, batch_index);

    std::vector<int> symbolic_eigenvalues = extract_symbolic_eigenvalues(psuedo_symbolic_eigenvalues, n, batch_size, digits, extraction_params.midpoint, extraction_params.midpoint_value, stagger, real_base, int_base);

    std::cout << "eigenvalues finished for batch: " << batch_index << std::endl;

    return symbolic_eigenvalues;
}

void process_batch(int batch_index, const std::vector<SymX>& A, int n, int batch_size, 
                   const std::vector<std::string>& symbols, const Dynamics& dynamics, 
                   const mpreal& real_base, int int_base, int digits, int stagger, 
                   const ExtractionParams& extraction_params, std::vector<int>& results_by_batch,
                   std::mutex& results_mutex) {
    std::vector<int> batch = calculate_eigenvalues_for_batch(A, n, batch_index, batch_size, symbols, 
                                                             dynamics, real_base, int_base, digits, 
                                                             stagger, extraction_params);
    
    std::lock_guard<std::mutex> lock(results_mutex);
    for (int eigen_index = 0; eigen_index < n; ++eigen_index) {
        for (int coeff_index = 0; coeff_index < batch_size; ++coeff_index) {
            results_by_batch[batch_index * batch_size * n + eigen_index * batch_size + coeff_index] = 
                batch[eigen_index * batch_size + coeff_index];
        }
    }
}

std::vector<std::vector<int>> hstack_coeffs_from_batches(const std::vector<int>& results_by_batch, 
                                              int n, int batch_size, int num_batches) {
    std::vector<std::vector<int>> combined_result(n, std::vector<int>(n, 0));

    for (int eigen_index = 0; eigen_index < n; ++eigen_index) {
        for (int coeff_index = 0; coeff_index < n; ++coeff_index) {
            int batch_for_coeff = coeff_index / batch_size;
            int coeff_in_batch = coeff_index % batch_size;
            
            if (batch_for_coeff < num_batches) {
                int flat_index = batch_for_coeff * (batch_size * n) + 
                                 eigen_index * batch_size + 
                                 coeff_in_batch;
                combined_result[eigen_index][coeff_index] = results_by_batch[flat_index];
            } else {
                std::cout << "Batch index out of range" << std::endl;
                exit(1);
            }
        }
    }

    return combined_result;
}

// symbolic eigenvalue solver for integer linear eigenvalue problems
// input matrix must be square and of a form that has an integer linear eigenvalue solution
std::vector<std::vector<int>> zle_eigs(const std::vector<std::vector<SymX>>& A, const std::vector<std::string>& symbols, int batch_size /*= -1*/, int stagger /*= 0*/, int base /*= 10*/) {
    int n = A.size();

    if (batch_size == -1) {
        batch_size = n;
    }

    int digits = batch_size * (stagger + 1);
    int precision_bits = digits_to_bits(digits + 2 * base, base);
    std::cout.precision(digits + 2 * base);
    mpreal::default_prec = precision_bits;
    mpcomplex::default_real_prec = precision_bits;
    mpcomplex::default_imag_prec = precision_bits;
    

    mpreal real_base = mpreal(base);
    int int_base = base;
    
    int num_batches = (n + batch_size - 1) / batch_size;

    ExtractionParams extraction_params = set_extraction_params(real_base, digits);
    Dynamics dynamics = set_dynamics(real_base, n, digits, stagger);

    // Flatten A into a 1D vector for better performance in the rest of the code
    std::vector<SymX> flattened_A(n * n);
    for (int i = 0; i < n; ++i) {
        std::copy(A[i].begin(), A[i].end(), flattened_A.begin() + i * n);
    }

    std::vector<int> results_by_batch(num_batches * batch_size * n);
    std::vector<std::thread> threads;
    std::mutex results_mutex;

    unsigned int num_threads = std::thread::hardware_concurrency();
    std::atomic<int> next_batch(0);

    auto thread_function = [&]() {
        while (true) {
            int batch_index = next_batch++;
            if (batch_index >= num_batches) break;
            
            process_batch(batch_index, flattened_A, n, batch_size, symbols, dynamics, 
                          real_base, int_base, digits, stagger, extraction_params, 
                          results_by_batch, results_mutex);
        }
    };

    for (unsigned int i = 0; i < num_threads; ++i) {
        threads.emplace_back(thread_function);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    std::vector<std::vector<int>> complete_eigens = hstack_coeffs_from_batches(results_by_batch, n, batch_size, num_batches);

    return complete_eigens;    
}

std::vector<std::string> get_symbols(int n) {
    std::vector<std::string> symbols(n);
    for (int i = 0; i < n; ++i) {
        symbols[i] = 'x' + std::to_string(i);
    }
    return symbols;
}

// create a SymX object representing a symbol with a coefficient
SymX symx(int i, std::string s /*= ""*/) {
    SymX res = SymX();
    res.coef = i;
    res.symbol = s;
    return res;
}
