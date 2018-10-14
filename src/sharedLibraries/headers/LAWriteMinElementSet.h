
#ifndef LA_WRITE_MINELEMENT_SET_H
#define LA_WRITE_MINELEMENT_SET_H

// by Binhang, Mar 2017

#include "WriteUserSet.h"
#include "LAMinElementOutputType.h"

using namespace pdb;
class LAWriteMinElementSet : public WriteUserSet<LAMinElementOutputType> {

public:
    ENABLE_DEEP_COPY

    LAWriteMinElementSet() {}

    // below constructor is not required, but if we do not call setOutput() here, we must call
    // setOutput() later to set the output set
    LAWriteMinElementSet(std::string dbName, std::string setName) {
        this->setOutput(dbName, setName);
    }
};


#endif
