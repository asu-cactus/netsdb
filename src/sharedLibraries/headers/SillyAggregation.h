#ifndef SILLY_AGG_H
#define SILLY_AGG_H

// by Jia, Mar 2017

#include "ClusterAggregateComp.h"
#include "DepartmentTotal.h"
#include "Employee.h"
#include "LambdaCreationFunctions.h"


using namespace pdb;

class SillyAggregation : public ClusterAggregateComp<DepartmentTotal, Employee, String, double> {

public:
    ENABLE_DEEP_COPY

    SillyAggregation() {}

    // the below constructor is NOT REQUIRED
    // user can also set output later by invoking the setOutput (std :: string dbName, std :: string
    // setName)  method
    SillyAggregation(std::string dbName, std::string setName) {
        this->setOutput(dbName, setName);
    }


    // the key type must have == and size_t hash () defined
    Lambda<String> getKeyProjection(Handle<Employee> aggMe) override {
        return makeLambdaFromMember(aggMe, department);
    }

    // the value type must have + defined
    Lambda<double> getValueProjection(Handle<Employee> aggMe) override {
        return makeLambdaFromMethod(aggMe, getSalary);
    }
};


#endif
