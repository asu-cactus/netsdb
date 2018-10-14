#ifndef SIMPLE_AGG_H
#define SIMPLE_AGG_H

// by Jia, Mar 2017

#include "ClusterAggregateComp.h"
#include "DepartmentTotal.h"
#include "SimpleEmployee.h"
#include "LambdaCreationFunctions.h"


using namespace pdb;

class SimpleAggregation
    : public ClusterAggregateComp<DepartmentTotal, SimpleEmployee, String, double> {

public:
    ENABLE_DEEP_COPY

    SimpleAggregation() {}

    // the key type must have == and size_t hash () defined
    Lambda<String> getKeyProjection(Handle<SimpleEmployee> aggMe) override {
        return makeLambdaFromMember(aggMe, department);
    }

    // the value type must have + defined
    Lambda<double> getValueProjection(Handle<SimpleEmployee> aggMe) override {
        return makeLambdaFromMember(aggMe, salary);
    }
};


#endif
