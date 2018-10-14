#ifndef BASEQUERY_H
#define BASEQUERY_H

#include "Lambda.h"
#include "Object.h"
#include "PDBString.h"

// PRELOAD %BaseQuery%

namespace pdb {

class BaseQuery : public Object {
public:
    ENABLE_DEEP_COPY

    BaseQuery() {}
    ~BaseQuery() {}
    virtual void toMap(std::map<std::string, pdb::GenericLambdaObjectPtr>& fillMe,
                       int& identifier) {
        std::cout << "Not expected in BaseQuery" << std::endl;
    }
};
}

#endif