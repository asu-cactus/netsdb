#ifndef TREE_RESULT_AGGREGATE_H
#define TREE_RESULT_AGGREGATE_H


#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "ClusterAggregateComp.h"
#include "TreeResult.h"

// PRELOAD %TreeResultAggregate%

namespace pdb {

class TreeResultAggregate

    : public ClusterAggregateComp<TreeResult, TreeResult, int, TreeResult> {

public:
    ENABLE_DEEP_COPY

    TreeResultAggregate(std::string dbName, std::string setName){
    
	this->setDatabaseName(dbName);
	this->setSetName(setName);
    
    }

    TreeResultAggregate() {}

    // the key type must have == and size_t hash () defined
    Lambda<int> getKeyProjection(Handle<TreeResult> aggMe) override {
        return makeLambdaFromMethod(aggMe, getKey);
    }

    // the value type must have + defined
    Lambda<TreeResult> getValueProjection(Handle<TreeResult> aggMe) override {
        return makeLambdaFromMethod(aggMe, getValue);
    }
};

}

#endif
