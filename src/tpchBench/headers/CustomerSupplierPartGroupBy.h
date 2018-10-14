#ifndef CustomerSupplierPartGroupBy_H
#define CustomerSupplierPartGroupBy_H

#include "ClusterAggregateComp.h"

#include "Lambda.h"
#include "LambdaCreationFunctions.h"

#include "PDBVector.h"
#include "PDBString.h"

#include "SupplierInfo.h"
#include "SupplierData.h"

using namespace pdb;

// template <class OutputClass, class InputClass, class KeyClass, class ValueClass>
class CustomerSupplierPartGroupBy
    : public ClusterAggregateComp<SupplierInfo, SupplierInfo, String, SupplierInfo> {

public:
    ENABLE_DEEP_COPY

    CustomerSupplierPartGroupBy() {}

    // the below constructor is NOT REQUIRED
    // user can also set output later by invoking the setOutput (std :: string dbName, std :: string
    // setName)  method
    CustomerSupplierPartGroupBy(std::string dbName, std::string setName) {
        this->setOutput(dbName, setName);
    }

    // the key type must have == and size_t hash () defined
    Lambda<String> getKeyProjection(Handle<SupplierInfo> aggMe) override {
        return makeLambda(aggMe, [](Handle<SupplierInfo>& aggMe) {
            return aggMe->supplierName;

        });
    }

    // the value type must have + defined
    Lambda<SupplierInfo> getValueProjection(Handle<SupplierInfo> aggMe) override {
        return makeLambda(aggMe, [](Handle<SupplierInfo>& aggMe) { return *aggMe; });
    }
};


#endif
