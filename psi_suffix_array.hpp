#pragma once

#include <vector>
#include <string>

#include "compresser.hpp"

struct Region
{
    int start = 0;
    int end = 0;
};

class PsiSuffixArray
{
public:
    PsiSuffixArray(const std::string& s, const std::vector<int> &suffix_array, int compress_step, int sample_step);

    void printMemorySize() const;

    int findPsiIndexForQuery(const std::string &query) const;
    int getTextIndexFromPsiIndex(int index) const;

private:
    std::array<Region, 256> regions;
    std::array<std::vector<Compresser>, 256> compressed_psi;
    std::vector<int> sampled_suffix_array;
    std::vector<unsigned char> sampled_chars;
    int compress_step;
    int sample_step;
    int psi_size;
    int sample_char_step = 128;

    void convertToPsi(const std::string &s, const std::vector<int> &suffix_array);
    void compressPsi(const std::vector<int> &psi);
    void sampleSuffixArray(const std::vector<int> &suffix_array);
    int getPsiValue(unsigned char c, int index) const;
    int getFirstCharForPsiIndex(int index) const;
};

