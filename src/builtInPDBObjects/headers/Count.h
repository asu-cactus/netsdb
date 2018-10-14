#ifndef PDB_COUNT_H
#define PDB_COUNT_H

// PRELOAD %Count%


#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "ClusterAggregateComp.h"
#include "SumResult.h"


/* The aggregation that counts the total number of inputs */
namespace pdb {

class Count
    : public ClusterAggregateComp<SumResult, Object, int, int> {

public:
    ENABLE_DEEP_COPY

    Count() {}

    // the below constructor is NOT REQUIRED
    // user can also set output later by invoking the setOutput (std :: string dbName, std :: string
    // setName)  method
    Count(std::string dbName, std::string setName) {
        this->setOutput(dbName, setName);
    }

    Lambda<int> getKeyProjection(Handle<Object> aggMe) override {
        return makeLambda(aggMe, [](Handle<Object>& aggMe) { return 0; });
    }

    Lambda<int> getValueProjection(Handle<Object> aggMe) override {
        return makeLambda(aggMe, [](Handle<Object>& aggMe) { return 1; });
    }
};

}

#endif
