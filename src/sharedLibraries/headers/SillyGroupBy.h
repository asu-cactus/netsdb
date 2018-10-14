#ifndef SILLY_GROUPBY_H
#define SILLY_GROUPBY_H

// by Jia, Mar 2017

#include "ClusterAggregateComp.h"
#include "Employee.h"
#include "Supervisor.h"
#include "LambdaCreationFunctions.h"
#include "DepartmentEmployeeAges.h"

using namespace pdb;

class SillyGroupBy : public ClusterAggregateComp<DepartmentEmployeeAges,
                                                 Supervisor,
                                                 String,
                                                 Handle<Map<String, Vector<int>>>> {

public:
    ENABLE_DEEP_COPY

    SillyGroupBy() {}

    // the below constructor is NOT REQUIRED
    // user can also set output later by invoking the setOutput (std :: string dbName, std :: string
    // setName)  method
    SillyGroupBy(std::string dbName, std::string setName) {
        this->setOutput(dbName, setName);
    }


    // the key type must have == and size_t hash () defined
    Lambda<String> getKeyProjection(Handle<Supervisor> aggMe) override {
        return makeLambdaFromMethod(aggMe, getDepartment);
    }

    // the value type must have + defined
    Lambda<Handle<Map<String, Vector<int>>>> getValueProjection(Handle<Supervisor> aggMe) override {
        return makeLambda(aggMe, [](Handle<Supervisor>& aggMe) {

            Handle<Map<String, Vector<int>>> ret = makeObject<Map<String, Vector<int>>>();
            String myKey = *(aggMe->getName());
            (*ret)[myKey].resize((aggMe->myGuys).size());
            for (int i = 0; i < (aggMe->myGuys).size(); i++) {
                (*ret)[myKey].push_back((aggMe->myGuys)[i]->getAge());
            }
            return ret;
        });
    }
};


#endif
