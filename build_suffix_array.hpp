#pragma once

#include <string>
#include <vector>

class SuffixArray
{
public:
    std::vector<int> suffix_array;
    std::string s;

    SuffixArray(const std::string &s);

    void printSuffixArray() const;
    void printMemorySize() const;

    int getIndex(const std::string &query) const;

private:
    void buildSuffixArray();
};