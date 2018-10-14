#ifndef LA_WRITE_MATRIXBLOCK_SET_H
#define LA_WRITE_MATRIXBLOCK_SET_H

// by Binhang, May 2017

#include "WriteUserSet.h"
//#include "BuiltInMatrixBlock.h"
#include "MatrixBlock.h"

using namespace pdb;
class LAWriteMatrixBlockSet : public WriteUserSet<MatrixBlock> {

public:
    ENABLE_DEEP_COPY

    LAWriteMatrixBlockSet() {}

    // below constructor is not required, but if we do not call setOutput() here, we must call
    // setOutput() later to set the output set
    LAWriteMatrixBlockSet(std::string dbName, std::string setName) {
        this->setOutput(dbName, setName);
    }
};


#endif
