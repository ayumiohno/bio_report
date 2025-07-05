#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

#include "build_suffix_array.hpp"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

// 2-digit radix sort for suffix array
void radixSortForSuffixArray(const vector<int> &rank, vector<int> &bucket, int step)
{
    // int max_rank = std::max((int)rank.size() + 1, 256);
    int max_rank = *std::max_element(rank.begin(), rank.end()) + 1;

    vector<int> count(max_rank, 0);
    vector<int> lsd_bucket(rank.size(), 0);

    for (int i = 0; i < rank.size(); i++)
    {
        int second_key = (i + step < rank.size()) ? rank[i + step] : 0;
        count[second_key]++;
    }

    for (int i = 1; i < max_rank; i++)
    {
        count[i] += count[i - 1];
    }

    for (int i = rank.size() - 1; i >= 0; i--)
    {
        int second_key = (i + step < rank.size()) ? rank[i + step] : 0;
        lsd_bucket[--count[second_key]] = i;
    }

    std::fill(count.begin(), count.end(), 0);
    for (int i = 0; i < rank.size(); i++)
    {
        count[rank[i]]++;
    }

    for (int i = 1; i < max_rank; i++)
    {
        count[i] += count[i - 1];
    }

    for (int i = rank.size() - 1; i >= 0; i--)
    {
        int index = lsd_bucket[i];
        bucket[--count[rank[index]]] = index;
    }
}

bool isSameRankWithPrev(const vector<int> &rank, const vector<int> &bucket, int index, int step, int n)
{
    if (rank[bucket[index]] != rank[bucket[index - 1]])
        return false;
    if (bucket[index] + step >= n || bucket[index - 1] + step >= n)
        return bucket[index] + step >= n && bucket[index - 1] + step >= n;
    return rank[bucket[index] + step] == rank[bucket[index - 1] + step];
}

// Compare the suffix at suffix_pos with the query string
// Returns:
// 0   if the suffix matches the query
// < 0 if the suffix is less than the query
// > 1 if the suffix is greater than the query
int compareSuffix(const string& s, int suffix_pos, const string& query) {
    int i = 0;
    while (i < query.size() && (suffix_pos + i) < s.size()) {
        if (s[suffix_pos + i] != query[i])
            return (s[suffix_pos + i] < query[i]) ? -1 : 1;
        i++;
    }
    if (i == query.size()) {
        if (suffix_pos + i < s.size() - 1) { 
            // ex) ippi$ and ipp
            return 2;
        }
        return 0;
    }
    return -1;
}

SuffixArray::SuffixArray(const string &s) : s(s)
{
    buildSuffixArray();
}

// Build suffix array using Manber-Myers algorithm
void SuffixArray::buildSuffixArray()
{
    int n = s.size();
    vector<int> rank(n, 0);
    vector<int> bucket(n, 0);

    for (int i = 0; i < n; i++)
    {
        rank[i] = s[i];
    }

    for (int step = 1; step < n; step *= 2)
    {
        radixSortForSuffixArray(rank, bucket, step);

        // Update ranks
        vector<int> next_rank(n, 0);
        next_rank[bucket[0]] = 0;
        for (int i = 1; i < n; i++)
        {
            next_rank[bucket[i]] = next_rank[bucket[i - 1]] + !isSameRankWithPrev(rank, bucket, i, step, n);
        }
        rank = next_rank;
    }
    suffix_array = bucket;
}

void SuffixArray::printSuffixArray() const
{
    cout << "index\tsa\tsuffix\n";
    for (int i = 0; i < suffix_array.size(); i++)
    {
        cout << i << ":\t" << suffix_array[i] << "\t" << s.substr(suffix_array[i]) << '\n';
    }
}

int SuffixArray::getIndex(const string &query) const
{
    int left = 0, right = suffix_array.size() - 1;
    int partial_match_index = -1;
    while (left <= right)
    {
        int mid = (left + right) / 2;
        int cmp = compareSuffix(s, suffix_array[mid], query);
        if (cmp == 0)
            return suffix_array[mid];
        else if (cmp < 0)
            left = mid + 1;
        else
            right = mid - 1;
        if (cmp == 2)
            partial_match_index = suffix_array[mid];
    }
    return partial_match_index; // Return the index if substring found, otherwise -1
}

void SuffixArray::printMemorySize() const
{
    cout << "Suffix array capacity: " << suffix_array.capacity() * sizeof(int) << " bytes" << endl;
    cout << "Suffix array size: " << suffix_array.size() * sizeof(int) << " bytes" << endl;
}
