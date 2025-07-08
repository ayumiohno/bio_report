#pragma once

#include <string>
#include <vector>

class SuffixArray
{
public:
    std::vector<int> suffix_array;
    std::string s;

    SuffixArray(const std::string &s);
    SuffixArray(const std::string &s, const std::vector<int> &suffix_array);

    void printMemorySize() const;

    int findTextIndexByQuery(const std::string &query) const;

private:
    void buildSuffixArray();
};