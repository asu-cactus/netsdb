#ifndef COUNT_AGGREGATION_H
#define COUNT_AGGREGATION_H

#include "ClusterAggregateComp.h"
#include "LambdaCreationFunctions.h"

#include "SupplierInfo.h"
#include "SumResult.h"

using namespace pdb;

// template <class OutputClass, class InputClass, class KeyClass, class ValueClass>

class CountAggregation : public ClusterAggregateComp<SumResult, SupplierInfo, int, int> {

public:
    ENABLE_DEEP_COPY

    CountAggregation() {}

    // the below constructor is NOT REQUIRED
    // user can also set output later by invoking the setOutput (std :: string dbName, std :: string
    // setName)  method
    CountAggregation(std::string dbName, std::string setName) {
        this->setOutput(dbName, setName);
    }


    // the key type must have == and size_t hash () defined
    Lambda<int> getKeyProjection(Handle<SupplierInfo> aggMe) override {
        return makeLambda(aggMe, [](Handle<SupplierInfo>& aggMe) { return 0; });
    }

    // the value type must have + defined
    Lambda<int> getValueProjection(Handle<SupplierInfo> aggMe) override {
        return makeLambda(aggMe, [](Handle<SupplierInfo>& aggMe) { return 1; });
    }
};
#endif
