#ifndef COUNT_CUSTOMER_AGGERGATION_H
#define COUNT_CUSTOMER_AGGERGATION_H

#include "ClusterAggregateComp.h"
#include "LambdaCreationFunctions.h"

#include "Customer.h"
#include "SumResult.h"

using namespace pdb;

// template <class OutputClass, class InputClass, class KeyClass, class ValueClass>

class CountCustomer : public ClusterAggregateComp<SumResult, Customer, int, int> {

public:
    ENABLE_DEEP_COPY

    CountCustomer() {}

    // the below constructor is NOT REQUIRED
    // user can also set output later by invoking the setOutput (std :: string dbName, std :: string
    // setName)  method
    CountCustomer(std::string dbName, std::string setName) {
        this->setOutput(dbName, setName);
    }


    // the key type must have == and size_t hash () defined
    Lambda<int> getKeyProjection(Handle<Customer> aggMe) override {
        return makeLambda(aggMe, [](Handle<Customer>& aggMe) { return 0; });
    }

    // the value type must have + defined
    Lambda<int> getValueProjection(Handle<Customer> aggMe) override {
        return makeLambda(aggMe, [](Handle<Customer>& aggMe) { return 1; });
    }
};
#endif
