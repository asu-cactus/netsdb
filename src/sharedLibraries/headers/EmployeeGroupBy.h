#ifndef EMPLOYEE_GROUPBY_H
#define EMPLOYEE_GROUPBY_H

// by Jia, Mar 2017

#include "ClusterAggregateComp.h"
#include "Employee.h"
#include "Supervisor.h"
#include "LambdaCreationFunctions.h"
#include "DepartmentEmployees.h"

using namespace pdb;

class EmployeeGroupBy : public ClusterAggregateComp<DepartmentEmployees,
                                                    Supervisor,
                                                    String,
                                                    Map<String, Vector<Handle<Employee>>>> {

public:
    ENABLE_DEEP_COPY

    EmployeeGroupBy() {}

    // the below constructor is NOT REQUIRED
    // user can also set output later by invoking the setOutput (std :: string dbName, std :: string
    // setName)  method
    EmployeeGroupBy(std::string dbName, std::string setName) {
        this->setOutput(dbName, setName);
    }


    // the key type must have == and size_t hash () defined
    Lambda<String> getKeyProjection(Handle<Supervisor> aggMe) override {
        return makeLambdaFromMethod(aggMe, getDepartment);
    }

    // the value type must have + defined
    Lambda<Map<String, Vector<Handle<Employee>>>> getValueProjection(
        Handle<Supervisor> aggMe) override {
        return makeLambda(aggMe, [](Handle<Supervisor>& aggMe) {

            Handle<Map<String, Vector<Handle<Employee>>>> ret =
                makeObject<Map<String, Vector<Handle<Employee>>>>();
            String myKey = *(aggMe->getName());
            (*ret)[myKey].resize((aggMe->myGuys).size());
            for (int i = 0; i < (aggMe->myGuys).size(); i++) {
                (*ret)[myKey].push_back((aggMe->myGuys)[i]);
            }
            return *ret;
        });
    }
};


#endif
