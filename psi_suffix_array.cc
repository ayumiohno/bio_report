#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <array>

#include "psi_suffix_array.hpp"

using std::cout;
using std::endl;

PsiSuffixArray::PsiSuffixArray(const std::string &s, const std::vector<int> &suffix_array, int compress_step, int sample_step)
    : compress_step(compress_step), sample_step(sample_step)
{
    sampleSuffixArray(suffix_array);
    convertToPsi(s, suffix_array);
}

void PsiSuffixArray::convertToPsi(const std::string &s, const std::vector<int> &sa)
{
    int n = s.size();
    psi_size = n;
    std::vector<int> inverse_sa(n);
    for (int i = 0; i < n; ++i)
        inverse_sa[sa[i]] = i;
    longest_idx = inverse_sa[0];
    std::vector<int> psi(n);

    sampled_chars.reserve((n + sample_step - 1) / sample_char_step + 1);
    for (int i = 1; i < n; ++i)
    {
        psi[i] = inverse_sa[sa[i] + 1];

        if (s[sa[i]] != s[sa[i - 1]])
        {
            regions[s[sa[i]]].start = i;
            regions[s[sa[i - 1]]].end = i - 1;
        }

        if ((i - 1) % sample_char_step == 0)
            sampled_chars.push_back(s[sa[(i - 1)]]);
    }
    psi[0] = -1;

    regions[s[sa[n - 1]]].end = n - 1;

    compressPsi(psi);
}

void PsiSuffixArray::compressPsi(const std::vector<int> &psi)
{
    for (int c = 0; c < 256; ++c)
    {
        int start = regions[c].start;
        int end = regions[c].end;
        if (start == 0 && end == 0)
            continue;
        compressed_psi[c].reserve((end - start + 1) / compress_step + 1);
        for (int i = start; i <= end; i += compress_step)
            compressed_psi[c].push_back(Compresser(psi, i, std::min(i + compress_step, end + 1)));
    }
}

void PsiSuffixArray::sampleSuffixArray(const std::vector<int> &suffix_array)
{
    int n = suffix_array.size();
    sampled_suffix_array.reserve((n + sample_step - 1) / sample_step);
    for (int i = 0; i < n; i += sample_step)
    {
        sampled_suffix_array.push_back(suffix_array[i]);
    }
}

int PsiSuffixArray::getPsi(unsigned char c, int index) const
{
    int index_in_region = index - regions[c].start;
    auto &comp = compressed_psi[c][index_in_region / compress_step];
    int local_index = index_in_region % compress_step;
    int value;
    if (!comp.getValue(value, local_index))
    {
        std::cerr << "Error: failed to get values for compressed psi. " << c << " " << index_in_region << std::endl;
        return 0;
    }
    return value;
}

int PsiSuffixArray::getCharRegion(int index) const
{
    for (int c = sampled_chars[index / sample_char_step]; c < 256; ++c)
    {
        if (regions[c].start <= index && index <= regions[c].end)
            return c;
    }
    return -1; // Not found
}

int PsiSuffixArray::getIndex(const std::string &query) const
{
    if (query.empty())
        return -1;

    int left = 1, right = psi_size - 1;
    int mid;
    int substring_index = -1;
    while (left <= right)
    {
        mid = (left + right) / 2;
        int cursor = mid;
        int i;
        for (i = 0; i < query.size(); ++i)
        {
            unsigned char c = getCharRegion(cursor);
            if (c < query[i])
            {
                left = mid + 1;
                break;
            }
            else if (c > query[i])
            {
                right = mid - 1;
                break;
            }
            cursor = getPsi((unsigned char)c, cursor);
        }
        if (cursor == 0 && i == query.size()) // Found
            return mid;
        if (i == query.size())
        {
            right = mid - 1;
            substring_index = mid;
        }
    }
    return substring_index;
}

int PsiSuffixArray::getSuffix(const std::string &query) const
{
    int index = getIndex(query);
    if (index == -1)
    {
        std::cerr << "Query not found: " << query << endl;
        return -1;
    }
    int cursor = index;
    int count = 0;
    while (cursor != 0)
    {
        if (cursor % sample_step == 0)
        {
            return sampled_suffix_array[cursor / sample_step] - count;
        }
        unsigned char c = getCharRegion(cursor);
        cursor = getPsi(c, cursor);
        count++;
    }
    return psi_size - count - 1; // Return the suffix index
}

void PsiSuffixArray::printPsi() const
{
    cout << "Longest suffix index: " << longest_idx << endl;

    cout << "Regions:" << endl;
    for (int i = 0; i < 256; ++i)
    {
        if (regions[i].start != 0 || regions[i].end != 0)
        {
            cout << static_cast<char>(i) << ": [" << regions[i].start << ", " << regions[i].end << "]" << endl;
        }
    }

    cout << "Compressed Psi:" << endl;
    for (int c = 0; c < 256; ++c)
    {
        if (!compressed_psi[c].empty())
        {
            cout << static_cast<char>(c) << ": ";
            for (auto &comp : compressed_psi[c])
            {
                std::vector<int> values;
                comp.getValues(values);
                for (const auto &value : values)
                {
                    cout << value << " ";
                }
            }
            cout << endl;
        }
    }
}

void PsiSuffixArray::printMemorySize() const
{
    size_t total_size = sizeof(regions);
    size_t total_capacity = total_size;

    total_size += sampled_suffix_array.size() * sizeof(int) + sampled_chars.size() * sizeof(unsigned char);
    total_capacity += sampled_suffix_array.capacity() * sizeof(int) + sampled_chars.capacity() * sizeof(unsigned char);

    int total_comp_size = sizeof(compressed_psi);
    int total_comp_capacity = sizeof(compressed_psi);
    for (const auto &comp_vec : compressed_psi)
    {
        total_comp_size += comp_vec.size() * sizeof(Compresser);
        total_comp_capacity += comp_vec.capacity() * sizeof(Compresser);

        for (const auto &c : comp_vec)
        {
            total_comp_size += c.getHeapSize();
            total_comp_capacity += c.getHeapCapacity();
        }
    }
    total_size += total_comp_size;
    total_capacity += total_comp_capacity;

    std::cout << "Psi memory size: " << total_size << " bytes" << std::endl;
    std::cout << "Psi memory capacity: " << total_capacity << " bytes" << std::endl;

    std::cout << "Sampled suffix array size: " << sampled_suffix_array.size() * sizeof(int) << " bytes" << std::endl;
    std::cout << "Sampled suffix array capacity: " << sampled_suffix_array.capacity() * sizeof(int) << " bytes" << std::endl;
    std::cout << "Compressed psi size: " << total_comp_size << " bytes" << std::endl;
    std::cout << "Compressed psi capacity: " << total_comp_capacity << " bytes" << std::endl;
}
