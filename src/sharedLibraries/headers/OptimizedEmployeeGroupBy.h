#ifndef OPTIMIZED_EMPLOYEE_GROUPBY_H
#define OPTIMIZED_EMPLOYEE_GROUPBY_H

// by Jia, Mar 2017

#include "ClusterAggregateComp.h"
#include "OptimizedEmployee.h"
#include "OptimizedSupervisor.h"
#include "LambdaCreationFunctions.h"
#include "OptimizedDepartmentEmployees.h"

using namespace pdb;

class OptimizedEmployeeGroupBy
    : public ClusterAggregateComp<OptimizedDepartmentEmployees,
                                  OptimizedSupervisor,
                                  String,
                                  Map<String, Vector<OptimizedEmployee>>> {

public:
    ENABLE_DEEP_COPY

    OptimizedEmployeeGroupBy() {}

    // the below constructor is NOT REQUIRED
    // user can also set output later by invoking the setOutput (std :: string dbName, std :: string
    // setName)  method
    OptimizedEmployeeGroupBy(std::string dbName, std::string setName) {
        this->setOutput(dbName, setName);
    }


    // the key type must have == and size_t hash () defined
    Lambda<String> getKeyProjection(Handle<OptimizedSupervisor> aggMe) override {
        return makeLambdaFromMethod(aggMe, getDepartment);
    }

    // the value type must have + defined
    Lambda<Map<String, Vector<OptimizedEmployee>>> getValueProjection(
        Handle<OptimizedSupervisor> aggMe) override {
        return makeLambda(aggMe, [](Handle<OptimizedSupervisor>& aggMe) {

            Map<String, Vector<OptimizedEmployee>> ret;
            Vector<OptimizedEmployee>& vec = ret[aggMe->getName()];
            Vector<OptimizedEmployee>& myGuys = aggMe->myGuys;
            size_t mySize = myGuys.size();
            vec.resize(mySize);
            for (int i = 0; i < mySize; i++) {
                vec.push_back(myGuys[i]);
            }
            return ret;
        });
    }
};


#endif
