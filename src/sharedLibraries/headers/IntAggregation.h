#ifndef INT_AGG_H
#define INT_AGG_H

// by Jia, Mar 2017

#include "ClusterAggregateComp.h"
#include "LambdaCreationFunctions.h"
#include "SumResult.h"

using namespace pdb;

class IntAggregation : public ClusterAggregateComp<SumResult, int, int, int> {

public:
    ENABLE_DEEP_COPY

    IntAggregation() {}

    // the below constructor is NOT REQUIRED
    // user can also set output later by invoking the setOutput (std :: string dbName, std :: string
    // setName)  method
    IntAggregation(std::string dbName, std::string setName) {
        this->setOutput(dbName, setName);
    }


    // the key type must have == and size_t hash () defined
    Lambda<int> getKeyProjection(Handle<int> aggMe) override {
        return makeLambda(aggMe, [](Handle<int>& aggMe) { return 0; });
    }

    // the value type must have + defined
    Lambda<int> getValueProjection(Handle<int> aggMe) override {
        return makeLambda(aggMe, [](Handle<int>& aggMe) {
            // return *aggMe;
            return 1;
        });
    }
};


#endif
