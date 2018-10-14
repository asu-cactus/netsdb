#ifndef DOUBLE_VECTOR_AGG_H
#define DOUBLE_VECTOR_AGG_H

// by Jia, May 2017

#include "ClusterAggregateComp.h"
#include "DoubleVector.h"
#include "DoubleVectorResult.h"
#include "LambdaCreationFunctions.h"


using namespace pdb;

class DoubleVectorAggregation
    : public ClusterAggregateComp<DoubleVectorResult, DoubleVector, int, DoubleVector> {

public:
    ENABLE_DEEP_COPY

    DoubleVectorAggregation() {}

    // the below constructor is NOT REQUIRED
    // user can also set output later by invoking the setOutput (std :: string dbName, std :: string
    // setName)  method
    DoubleVectorAggregation(std::string dbName, std::string setName) {
        this->setOutput(dbName, setName);
    }


    // the key type must have == and size_t hash () defined
    Lambda<int> getKeyProjection(Handle<DoubleVector> aggMe) override {
        return makeLambda(aggMe, [](Handle<DoubleVector>& aggMe) { return 0; });
    }

    // the value type must have + defined
    Lambda<DoubleVector> getValueProjection(Handle<DoubleVector> aggMe) override {
        return makeLambda(aggMe, [](Handle<DoubleVector>& aggMe) { return *aggMe; });
    }
};


#endif
