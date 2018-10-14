#ifndef MATRIX_META_H
#define MATRIX_META_H

#include "Object.h"

// By Binhang Yuan, May 2017

class MatrixMeta : public pdb::Object {
public:
    ENABLE_DEEP_COPY

    ~MatrixMeta() {}
    MatrixMeta() {}

    int blockRowIndex;
    int blockColIndex;

    int totalRows = 0;
    int totalCols = 0;


    bool operator==(const MatrixMeta& other) const {
        if (blockRowIndex == other.blockRowIndex && blockColIndex == other.blockColIndex) {
            return true;
        } else {
            return false;
        }
    }

    size_t hash() const {
        return 10000 * blockRowIndex + blockColIndex;
    }
};

#endif
