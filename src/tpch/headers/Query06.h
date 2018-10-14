#ifndef QUERY06_H
#define QUERY06_H


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
#include "DoubleSumResult.h"

using namespace pdb;

namespace tpch {


class Q06LineItemSelection : public SelectionComp<LineItem, LineItem> {


public:

    ENABLE_DEEP_COPY

    Q06LineItemSelection () {}


    Lambda<bool> getSelection(Handle<LineItem> checkMe) override {
        return makeLambda(checkMe,
           [&](Handle<LineItem>& checkMe) { 
                LineItem me = *checkMe;
                if ((strcmp(me.l_shipdate.c_str(), "1994-01-01") >= 0) &&
                    (strcmp(me.l_shipdate.c_str(), "1995-10-01") < 0) &&
                    (me.l_discount >= 0.05) && (me.l_discount <= 0.07) &&
                    (me.l_quantity < 24)) {
                    return true; 
                } else {
                    return false;
                }
           });
    }

    Lambda<Handle<LineItem>> getProjection(Handle<LineItem> checkMe) override {
        return makeLambda(checkMe, [](Handle<LineItem>& checkMe) { return checkMe; });
    }


};



class Q06Agg : public ClusterAggregateComp<DoubleSumResult,
                                               LineItem,
                                               int,
                                               double> {

public:

    ENABLE_DEEP_COPY


    Q06Agg () {}

    Lambda<int> getKeyProjection(Handle<LineItem> aggMe) override {
         return makeLambda(aggMe, [](Handle<LineItem>& aggMe) { return 0; });
    }

    Lambda<double> getValueProjection (Handle<LineItem> aggMe) override {
         return makeLambda(aggMe, [](Handle<LineItem>& aggMe) {
             return aggMe->l_extendedprice * aggMe->l_discount;
         });
    }


};



}

#endif
