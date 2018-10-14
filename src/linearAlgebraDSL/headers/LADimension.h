#ifndef LA_DIMENSION_H
#define LA_DIMENSION_H

// by Binhang, June 2017

typedef struct LADimension {
    int blockRowSize;  // Size for each block
    int blockColSize;
    int blockRowNum;  // Number of blocks
    int blockColNum;

    LADimension() : blockRowSize(0), blockColSize(0), blockRowNum(0), blockColNum(0) {}
    LADimension(int rs, int cs, int rn, int cn)
        : blockRowSize(rs), blockColSize(cs), blockRowNum(rn), blockColNum(cn) {}

    LADimension transpose() {
        LADimension T;
        T.blockRowSize = this->blockColSize;
        T.blockColSize = this->blockRowSize;
        T.blockRowNum = this->blockColNum;
        T.blockColNum = this->blockRowNum;
        return T;
    }

    LADimension& operator=(const LADimension& other) {
        blockRowSize = other.blockRowSize;
        blockColSize = other.blockColSize;
        blockRowNum = other.blockRowNum;
        blockColNum = other.blockColNum;
        return *this;
    }

    bool operator==(const LADimension& other) {
        return blockRowSize == other.blockRowSize && blockColSize == other.blockColSize &&
            blockRowNum == other.blockRowNum && blockColNum == other.blockColNum;
    }

    bool operator!=(const LADimension& other) {
        return blockRowSize != other.blockRowSize || blockColSize != other.blockColSize ||
            blockRowNum != other.blockRowNum || blockColNum != other.blockColNum;
    }
} LADimension;


#endif