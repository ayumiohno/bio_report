#pragma once

#include <vector>

// Compresser for psi-index
class Compresser {
    public:
        Compresser(const std::vector<int>& values, int start, int end);
        bool getValue(int& value, int index) const;

        int getHeapSize() const {
            return compressed_diffs.size() * sizeof(uint8_t);
        }

    private:
        int first_value;
        std::vector<uint8_t> compressed_diffs;

        void writeBit(bool bit, int& pos);
        void writeDiffs(const std::vector<int>& diffs);
        bool readBit(int& pos, bool& bit) const;
};