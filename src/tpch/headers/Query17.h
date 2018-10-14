#ifndef QUERY17_H
#define QUERY17_H


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
#include "Avg.h"
#include "DoubleSumResult.h"
#include "AvgResult.h"

using namespace pdb;

namespace tpch {


class Q17PartSelection : public SelectionComp<Part, Part> {

private:

    String p_brand;
    String p_container;

public:

    ENABLE_DEEP_COPY

    Q17PartSelection () {}

    Q17PartSelection (std::string p_brand, std::string p_container) {
       this->p_brand = p_brand;
       this->p_container = p_container;
    }

    Lambda<bool> getSelection(Handle<Part> checkMe) override {
        return makeLambda(checkMe,
           [&](Handle<Part>& checkMe) {
                if ((checkMe->p_brand == p_brand) && 
                      (checkMe->p_container == p_container)) {
                    return true;
                } else {
                    return false;
                }
           });
    }

    Lambda<Handle<Part>> getProjection(Handle<Part> checkMe) override {
        return makeLambda(checkMe, [](Handle<Part>& checkMe) { return checkMe; });
    }


}; 


class Q17JoinedPartLineItem : public Object {

public:

    ENABLE_DEEP_COPY

    Q17JoinedPartLineItem () {}

    Q17JoinedPartLineItem (Part& part, LineItem& lineItem) {

        this->p_brand = part.p_brand;
        this->p_container = part.p_container;
        this->l_partkey = lineItem.l_partkey;
        this->l_extendedprice = lineItem.l_extendedprice;
        this->l_quantity = lineItem.l_quantity;

    }
    

    String p_brand;

    String p_container;

    int l_partkey;

    double l_extendedprice;

    double l_quantity;


};



class Q17PartLineItemJoin : public JoinComp<Q17JoinedPartLineItem, Part, LineItem> {

public:

    ENABLE_DEEP_COPY

    Q17PartLineItemJoin () {}

    Lambda<bool> getSelection (Handle<Part> in1, Handle<LineItem> in2) override {
        return makeLambdaFromMember(in1, p_partkey) == makeLambdaFromMember(in2,
           l_partkey);

    }

    Lambda<Handle<Q17JoinedPartLineItem>> getProjection(Handle<Part> in1, Handle<LineItem> in2) override {
        return makeLambda(in1, in2, [](Handle<Part>& in1, Handle<LineItem>& in2) {
            Handle<Q17JoinedPartLineItem> ret = makeObject<Q17JoinedPartLineItem>(*in1, *in2);
            return ret;
        });
    }


};


class Q17PartLineItemIdentitySelection : public SelectionComp<Q17JoinedPartLineItem, Q17JoinedPartLineItem> {

public:

    ENABLE_DEEP_COPY

    Q17PartLineItemIdentitySelection () {}

    Lambda<bool> getSelection (Handle<Q17JoinedPartLineItem> checkMe) override {
        return makeLambda(checkMe, [](Handle<Q17JoinedPartLineItem> & checkMe) {
             return true;
         });
    }

    Lambda<Handle<Q17JoinedPartLineItem>> getProjection (Handle<Q17JoinedPartLineItem> checkMe) override {
        return makeLambda(checkMe,  [](Handle<Q17JoinedPartLineItem> & checkMe) {
             return checkMe;
        });
    }

};



class Q17LineItemAvgQuantity : public ClusterAggregateComp<AvgResult,
                                               Q17JoinedPartLineItem,
                                               int,
                                               Avg> {

public:

    ENABLE_DEEP_COPY


    Q17LineItemAvgQuantity () {}

    Lambda<int> getKeyProjection(Handle<Q17JoinedPartLineItem> aggMe) override {
         return makeLambdaFromMember(aggMe, l_partkey);
    }

    Lambda<Avg> getValueProjection (Handle<Q17JoinedPartLineItem> aggMe) override {
         return makeLambda(aggMe, [](Handle<Q17JoinedPartLineItem>& aggMe) {
             Avg ret(0.2 * aggMe->l_quantity, 1);
             return ret;
         });
    }


};


class Q17PartLineItemAvgJoin : public JoinComp<double, Q17JoinedPartLineItem, AvgResult> {

public:

    ENABLE_DEEP_COPY

    Q17PartLineItemAvgJoin () {}

    Lambda<bool> getSelection (Handle<Q17JoinedPartLineItem> in1, Handle<AvgResult> in2) override {
        return makeLambdaFromMember(in1, l_partkey) == makeLambdaFromMember(in2,
           key) && makeLambda(in1, in2, [](Handle<Q17JoinedPartLineItem>& in1, Handle<AvgResult>& in2) {
                   if (in1->l_quantity < in2->getAvg()) {
                       return true;
                   } else {
                       return false;
                   }
               });

    }

    Lambda<Handle<double>> getProjection(Handle<Q17JoinedPartLineItem> in1, Handle<AvgResult> in2) override {
        return makeLambda(in1, in2, [](Handle<Q17JoinedPartLineItem>& in1, Handle<AvgResult>& in2) {
            Handle<double> ret = makeObject<double>(in1->l_extendedprice);
            return ret;
        });
    }


};


class Q17PriceSum : public ClusterAggregateComp<DoubleSumResult,
                                               double,
                                               int,
                                               double> {

public:

    ENABLE_DEEP_COPY


    Q17PriceSum () {}

    Lambda<int> getKeyProjection(Handle<double> aggMe) override {
         return makeLambda(aggMe, [](Handle<double> & aggMe) {
              return 0;
          });
    }

    Lambda<double> getValueProjection (Handle<double> aggMe) override {
         return makeLambda(aggMe, [](Handle<double>& aggMe) {
             return *aggMe;
         });
    }


};
}

#endif
