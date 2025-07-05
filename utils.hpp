#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

bool readFile(const std::string& filename, std::string& content) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return false;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    content = ss.str();
    return true;
}

std::string normalize_to_ascii(const std::string& input) {
    std::string result;
    for (unsigned char c : input) {
        if (c >= 32 && c <= 126)
            result += c;
    }
    return result;
}

bool checkSuffixArray(const std::string& s, const std::vector<int>& sa) {
    for (int i = 1; i < sa.size(); ++i) {
        if (s.substr(sa[i - 1]) > s.substr(sa[i])) {
            std::cerr << "Mismatch at " << i << ": "
                 << s.substr(sa[i - 1], 20) << " > "
                 << s.substr(sa[i], 20) << std::endl;
            return false;
        }
    }
    return true;
}

int getLogN(int n) {
    if (n <= 1) return 1;

    double log2n = std::log2(n);
    double loglog2n = std::log2(log2n);
    int k = static_cast<int>(std::round(loglog2n));
    return 1 << k;
}
