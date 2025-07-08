#include "compresser.hpp"

// Compresser for psi-index
// compress the values[start:end]
Compresser::Compresser(const std::vector<int> &values, int start, int end)
{
    first_value = values[start];
    std::vector<int> diffs;
    diffs.reserve(end - start - 1);
    for (int i = start + 1; i < end; i++)
    {
        diffs.push_back(values[i] - values[i - 1]);
    }
    writeDiffs(diffs);
}

void Compresser::writeBit(bool bit, int &pos)
{
    if (compressed_diffs.empty() || pos % 8 == 0)
        compressed_diffs.push_back(0);
    if (bit)
        compressed_diffs.back() |= (1ULL << (pos % 8));
    ++pos;
}

// write the diffs by gamma coding
void Compresser::writeDiffs(const std::vector<int> &diffs)
{
    int pos = 0;
    for (int x : diffs)
    {
        int length = 32 - __builtin_clz(x); // log2(x) + 1
        for (int i = 0; i < length - 1; ++i)
            writeBit(0, pos);
        writeBit(1, pos);
        for (int i = length - 2; i >= 0; --i)
            writeBit((x >> i) & 1, pos);
    }
}

bool Compresser::readBit(int &pos, bool &bit) const
{
    if (pos >= compressed_diffs.size() * 8)
        return false; // EOF
    bit = (compressed_diffs[pos / 8] >> (pos % 8)) & 1;
    ++pos;
    return true;
}

bool Compresser::getValue(int &value, int index) const
{
    int pos = 0;
    value = first_value;
    for (int i = 0; i < index; ++i)
    {
        bool bit;
        int length = 1;
        while (true)
        {
            if (!readBit(pos, bit))
                return false;
            if (bit)
                break;
            ++length;
        }
        int x = 1 << (length - 1);
        for (int i = length - 2; i >= 0; --i)
        {
            if (!readBit(pos, bit))
                return false;
            if (bit)
                x |= (1 << i);
        }
        value += x;
    }
    return true;
}
