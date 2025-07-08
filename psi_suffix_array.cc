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

/// Converts a suffix array into a ψ-array.
/// Also records character-region ranges and sampled characters for fast lookup.
void PsiSuffixArray::convertToPsi(const std::string &s, const std::vector<int> &sa)
{
    int n = s.size();
    psi_size = n;
    std::vector<int> inverse_sa(n);
    for (int i = 0; i < n; ++i)
        inverse_sa[sa[i]] = i;
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

/// Stores compressed ψ values in blocks of `compress_step` for each character.
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

/// Samples the suffix array every `sample_step` entries to allow partial reconstruction.
/// Sampled values are stored in `sampled_suffix_array`.
void PsiSuffixArray::sampleSuffixArray(const std::vector<int> &suffix_array)
{
    int n = suffix_array.size();
    sampled_suffix_array.reserve((n + sample_step - 1) / sample_step);
    for (int i = 0; i < n; i += sample_step)
    {
        sampled_suffix_array.push_back(suffix_array[i]);
    }
}

int PsiSuffixArray::getPsiValue(unsigned char c, int index) const
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

/// Given a ψ index, returns the corresponding first character of the suffix
/// (inferred from character region `regions` using `sampled_chars`).
int PsiSuffixArray::getFirstCharForPsiIndex(int index) const
{
    for (int c = sampled_chars[index / sample_char_step]; c < 256; ++c)
    {
        if (regions[c].start <= index && index <= regions[c].end)
            return c;
    }
    return -1;
}

/// Searches for the query string in the ψ-array using binary search.
/// Traverses ψ links character by character.
/// Returns the ψ index corresponding to the matching suffix, or -1 if not found.
int PsiSuffixArray::findPsiIndexForQuery(const std::string &query) const
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
            unsigned char c = getFirstCharForPsiIndex(cursor);
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
            cursor = getPsiValue((unsigned char)c, cursor);
        }
        if (cursor == 0 && i == query.size()) // Found
            return mid;
        if (i == query.size()) // Partial match
        {
            right = mid - 1;
            substring_index = mid;
        }
    }
    return substring_index;
}

/// Recovers the original text index corresponding to a given ψ index
/// by traversing ψ backwards until reaching a sampled suffix array entry.
/// Returns the original suffix start position in `s`.
int PsiSuffixArray::getTextIndexFromPsiIndex(int index) const
{
    int cursor = index;
    int count = 0;
    while (cursor != 0)
    {
        if (cursor % sample_step == 0)
        {
            return sampled_suffix_array[cursor / sample_step] - count;
        }
        unsigned char c = getFirstCharForPsiIndex(cursor);
        cursor = getPsiValue(c, cursor);
        count++;
    }
    return psi_size - count - 1;
}

void PsiSuffixArray::printMemorySize() const
{
    size_t total_size = sizeof(regions);

    total_size += sampled_suffix_array.size() * sizeof(int) + sampled_chars.size() * sizeof(unsigned char);

    int total_comp_size = sizeof(compressed_psi);
    for (const auto &comp_vec : compressed_psi)
    {
        total_comp_size += comp_vec.size() * sizeof(Compresser);

        for (const auto &c : comp_vec)
        {
            total_comp_size += c.getHeapSize();
        }
    }
    total_size += total_comp_size;

    cout << total_size << " " << total_comp_size << " " << sampled_suffix_array.size() * sizeof(int) << endl;
}
