#ifndef QUERY04_H
#define QUERY04_H


#include "TPCHSchema.h"
#include "PDBDebug.h"
#include "PDBString.h"
#include "Query.h"
#include "Lambda.h"
#include "PDBClient.h"
#include "DataTypes.h"
#include "InterfaceFunctions.h"
#include "Handle.h"
#include "LambdaCreationFunctions.h"
#include "UseTemporaryAllocationBlock.h"
#include "Pipeline.h"
#include "VectorSink.h"
#include "HashSink.h"
#include "MapTupleSetIterator.h"
#include "VectorTupleSetIterator.h"
#include "ComputePlan.h"
#include "QueryOutput.h"
#include "SelectionComp.h"
#include "JoinComp.h"
#include "ClusterAggregateComp.h"


using namespace pdb;

namespace tpch {

class Q04OrderSelection : public SelectionComp<Order, Order> {


public:

    ENABLE_DEEP_COPY

    Q04OrderSelection () {}


    Lambda<bool> getSelection(Handle<Order> checkMe) override {
        return makeLambda(checkMe,
           [&](Handle<Order>& checkMe) { 

                if ((strcmp(checkMe->o_orderdate.c_str(), "1993-07-01") >= 0) &&
                    (strcmp(checkMe->o_orderdate.c_str(), "1993-10-01") < 0)) {
                    return true; 
                } else {
                    return false;
                }
           });
    }

    Lambda<Handle<Order>> getProjection(Handle<Order> checkMe) override {
        return makeLambda(checkMe, [](Handle<Order>& checkMe) { return checkMe; });
    }


};


class Q04Join : public JoinComp<Order, Order, LineItem> {

public:

    ENABLE_DEEP_COPY

    Q04Join () {}

    Lambda<bool> getSelection (Handle<Order> in1, Handle<LineItem> in2) override {
        return makeLambdaFromMember(in1, o_orderkey) == makeLambdaFromMember(in2,
           l_orderkey);
         
    }
    
    Lambda<Handle<Order>> getProjection(Handle<Order> in1, Handle<LineItem> in2) override {
        return makeLambda(in1, in2, [](Handle<Order>& in1, Handle<LineItem>& in2) { return in1; });
    }


};

class Q04AggOut : public Object {

public:

    ENABLE_DEEP_COPY

    String o_orderpriority;
    int count;

    Q04AggOut () {}

    String & getKey() {
        return o_orderpriority;
    }

    int & getValue() {
        return count;
    }

};



class Q04Agg : public ClusterAggregateComp<Q04AggOut,
                                               Order,
                                               String,
                                               int> {

public:

    ENABLE_DEEP_COPY


    Q04Agg () {}

    Lambda<String> getKeyProjection(Handle<Order> aggMe) override {
         return makeLambdaFromMember(aggMe, o_orderpriority);
    }

    Lambda<int> getValueProjection (Handle<Order> aggMe) override {
         return makeLambda(aggMe, [](Handle<Order>& aggMe) {
             return 1;
         });
    }


};



}

#endif
