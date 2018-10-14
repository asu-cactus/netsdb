
#ifndef LA_WRITE_MAXELEMENT_SET_H
#define LA_WRITE_MAXELEMENT_SET_H

// by Binhang, Mar 2017

#include "WriteUserSet.h"
#include "LAMaxElementOutputType.h"

using namespace pdb;
class LAWriteMaxElementSet : public WriteUserSet<LAMaxElementOutputType> {

public:
    ENABLE_DEEP_COPY

    LAWriteMaxElementSet() {}

    // below constructor is not required, but if we do not call setOutput() here, we must call
    // setOutput() later to set the output set
    LAWriteMaxElementSet(std::string dbName, std::string setName) {
        this->setOutput(dbName, setName);
    }
};


#endif
