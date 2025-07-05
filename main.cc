#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <random>

#include "build_suffix_array.hpp"
#include "psi_suffix_array.hpp"
#include "utils.hpp"

int main()
{
    std::string filename = "books/2600-0.txt"; //"books/98-0.txt"; // "books/11-0.txt" or "books/2600-0.txt"
    // std::string filename = "random_chars.txt"; // Example file, change as needed
    std::string text;
    if (!readFile(filename, text))
    {
        std::cerr << "Error: Could not read file " << filename << std::endl;
        return 1;
    }
    text = normalize_to_ascii(text);
    // text = "mississippi";

    std::cout << text.size() << " characters read from file." << std::endl;

    text += "\0x3";

    SuffixArray sa(text);
    sa.printMemorySize();

    // checkSuffixArray(text, sa.suffix_array);

    int log_n = getLogN(text.size());
    std::cout << "n_log_n: " << log_n << std::endl;

    PsiSuffixArray psi(text, sa.suffix_array, 32, 4);
    psi.printMemorySize();

    std::random_device rnd;
    std::mt19937 mt(rnd());

    double sa_ave_time = 0.0;
    double psi_ave_time = 0.0;

    for (int i = 0; i < 100; i++)
    {
        int query_size = std::uniform_int_distribution<>(4, 256)(mt);
        std::string query = text.substr(std::uniform_int_distribution<>(0, text.size() - query_size)(mt), query_size);

        auto start = std::chrono::high_resolution_clock::now();
        int sa_result = sa.getIndex(query);
        sa_ave_time += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count();
        if (text.substr(sa_result, query.size()) != query)
        {
            std::cerr << "Error: SuffixArray index mismatch for query '" << query << "'." << std::endl;
        }

        start = std::chrono::high_resolution_clock::now();
        int psi_result = psi.getSuffix(query);
        psi_ave_time += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count();
        if (psi_result == -1 || text.substr(psi_result, query.size()) != query)
        {
            std::cerr << "Error: PsiSuffixArray index mismatch for query '" << query << "'." << std::endl;
        }
    }
    sa_ave_time /= 100.0;
    psi_ave_time /= 100.0;

    std::cout << "Average time for SuffixArray: " << sa_ave_time << " microseconds" << std::endl;
    std::cout << "Average time for PsiSuffixArray: " << psi_ave_time << " microseconds" << std::endl;
    return 0;
}
