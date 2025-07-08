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
    std::string filename = "100MB_random_chars.txt";
    // std::string filename = "wiki_100MB.txt";
    std::string text;
    if (!readFile(filename, text))
    {
        std::cerr << "Error: Could not read file " << filename << std::endl;
        return 1;
    }
    text = normalize_to_ascii(text);
    text += "\0x3";

    // std::cout << text.size() << " characters read from file." << std::endl;
    // std::random_device rnd;
    // std::mt19937 mt(rnd());
    // int query_size = 128;
    // for (int i = 0; i < 100; ++i)
    // {
    //     std::string query = text.substr(std::uniform_int_distribution<>(0, text.size() - query_size)(mt), query_size);
    //     std::cout << query << std::endl;
    // }

    std::vector<int> suffix_array;
    suffix_array.reserve(text.size());
    if (!readSuffixArray("100MB_random_chars_sa.txt", suffix_array))
    {
        std::cerr << "Error: Could not read suffix array file." << std::endl;
        return 1;
    }
    else
    {
        std::cout << "Suffix array read successfully." << std::endl;
    }

    // SuffixArray sa(text);
    SuffixArray sa(text, suffix_array);
    // checkSuffixArray(text, sa.suffix_array);

    // dump suffix array to file
    // std::ofstream sa_file("100MB_random_chars_sa.txt");
    // if (sa_file.is_open())
    // {
    //     for (const auto &index : sa.suffix_array)
    //     {
    //         sa_file << index << "\n";
    //     }
    //     sa_file.close();
    // }

    std::array<std::string, 100> queries;
    for (int i = 0; i < 100; ++i)
    {
        std::cin >> queries[i];
    }

    for (int epoch = 0; epoch < 5; ++epoch)
    {
        {
            double ave_time = 0.0;
            for (int i = 0; i < 20; ++i)
            {
                std::string query = queries[i + epoch * 20];
                auto start = std::chrono::high_resolution_clock::now();
                int sa_result = sa.findTextIndexByQuery(query);
                if (i >= 10)
                    ave_time += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count();
                if (sa_result == -1 || text.substr(sa_result, query.size()) != query)
                {
                    std::cerr << "Error: SuffixArray index mismatch for query '" << query << "'." << std::endl;
                }
            }
            std::cout << ave_time / 10.0 << " ";
            sa.printMemorySize();
        }

        std::array<std::pair<int, int>, 6> params = {
            std::make_pair(8, 16),
            std::make_pair(16, 16),
            std::make_pair(32, 16),
            std::make_pair(64, 16),
            std::make_pair(128, 16),
            std::make_pair(256, 16),
        };
        std::cout << "compress_step sample_step find_time sa_time psi_size psi_suffixarray_size psi_compressed_size" << std::endl;
        for (auto [compress_step, sample_step] : params)
        {
            double find_ave_time = 0.0;
            double sa_ave_time = 0.0;
            PsiSuffixArray psi(text, sa.suffix_array, compress_step, sample_step);
            for (int i = 0; i < 20; ++i)
            {
                std::string query = queries[i + epoch * 20];
                auto start = std::chrono::high_resolution_clock::now();
                int psi_index = psi.findPsiIndexForQuery(query);
                if (i >= 10)
                    find_ave_time += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count();
                start = std::chrono::high_resolution_clock::now();
                int sa_result = psi.getTextIndexFromPsiIndex(psi_index);
                if (i >= 10)
                    sa_ave_time += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count();
                if (sa_result == -1 || text.substr(sa_result, query.size()) != query)
                {
                    std::cerr << "Error: SuffixArray index mismatch for query '" << query << "'." << std::endl;
                }
            }
            std::cout << compress_step << " " << sample_step << " "
                      << find_ave_time / 10.0 << " " << sa_ave_time / 10.0 << " ";
            psi.printMemorySize();
        }
    }
}
