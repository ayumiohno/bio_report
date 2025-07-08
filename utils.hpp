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

bool readSuffixArray(const std::string& filename, std::vector<int>& suffix_array) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return false;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    std::istringstream ss(buffer.str());
    std::string line;
    while (std::getline(ss, line)) {
        if (!line.empty()) {
            try {
                suffix_array.push_back(std::stoi(line));
            } catch (const std::invalid_argument& e) {
                std::cerr << "Error: Invalid suffix array entry '" << line << "'" << std::endl;
                return false;
            }
        }
    }
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
