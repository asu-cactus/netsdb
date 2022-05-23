
#ifndef VECTORFLOAT_WRITER_H
#define VECTORFLOAT_WRITER_H

#include "WriteUserSet.h"
#include "PDBVector.h"

// PRELOAD %VectorFloatWriter%

namespace pdb {
class VectorFloatWriter : public WriteUserSet<Vector<float>> {

public:
    ENABLE_DEEP_COPY

    VectorFloatWriter() {}

    VectorFloatWriter(std::string dbName, std::string setName) {
        setDatabaseName(dbName);
        setSetName(setName);
    }
};
}

#endif
