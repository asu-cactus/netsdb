
#ifndef VECTORDOUBLE_WRITER_H
#define VECTORDOUBLE_WRITER_H

#include "WriteUserSet.h"
#include "PDBVector.h"

// PRELOAD %VectorDoubleWriter%

namespace pdb {
class VectorDoubleWriter : public WriteUserSet<Vector<float>> {

public:
    ENABLE_DEEP_COPY

    VectorDoubleWriter() {}

    VectorDoubleWriter(std::string dbName, std::string setName) {
        setDatabaseName(dbName);
        setSetName(setName);
    }
};

}
#endif
