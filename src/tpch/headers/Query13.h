#ifndef QUERY13_H
#define QUERY13_H


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

class Q13OrderSelection : public SelectionComp<Order, Order> {

private:

   String word1;
   String word2;

public:

    ENABLE_DEEP_COPY

    Q13OrderSelection () {}

    Q13OrderSelection (std::string word1, std::string word2) {
        this->word1 = word1;
        this->word2 = word2;
    }

    Lambda<bool> getSelection(Handle<Order> checkMe) override {
        return makeLambda(checkMe,
           [&](Handle<Order>& checkMe) {
                std::string o_comment = checkMe->o_comment;
                std::string::size_type n = o_comment.find(this->word1, 0);
                if (n == std::string::npos) {
                    return false;
                }
                n = o_comment.find(this->word2, n+sizeof(this->word1));
                if (n == std::string::npos) {
                    return false;
                }
                return true;

           });
    }

    Lambda<Handle<Order>> getProjection(Handle<Order> checkMe) override {
        return makeLambda(checkMe, [](Handle<Order>& checkMe) { return checkMe; });
    }


};

class Q13CustomerOrders : public Object {

public:

    ENABLE_DEEP_COPY

    Q13CustomerOrders () {}

    Q13CustomerOrders (int o_orderkey, int c_custkey) {
        this->o_orderkey = o_orderkey;
        this->c_custkey = c_custkey;
    }

    int o_orderkey;

    int c_custkey;

};

class Q13CustomerOrderJoin : public JoinComp<Q13CustomerOrders, Customer, Order> {

public:

    ENABLE_DEEP_COPY

    Q13CustomerOrderJoin () {}

    Lambda<bool> getSelection (Handle<Customer> in1, Handle<Order> in2) override {
        return makeLambdaFromMember(in1, c_custkey) == makeLambdaFromMember(in2,
           o_custkey);
         
    }
    
    Lambda<Handle<Q13CustomerOrders>> getProjection(Handle<Customer> in1, Handle<Order> in2) override {
        return makeLambda(in1, in2, [](Handle<Customer>& in1, Handle<Order>& in2) {
            Handle<Q13CustomerOrders> ret = makeObject<Q13CustomerOrders>(in2->o_orderkey, 
               in1->c_custkey);
            return ret;
        });
    }


};


class Q13CountResult : public Object {

public:

    int key;
    int value;

    ENABLE_DEEP_COPY

    Q13CountResult () {}

    int & getKey() {
        return key;
    }

    int & getValue() {
        return value;
    }

};


class Q13OrdersPerCustomer : public ClusterAggregateComp<Q13CountResult,
                                               Q13CustomerOrders,
                                               int,
                                               int> {

public:

    ENABLE_DEEP_COPY


    Q13OrdersPerCustomer () {}

    Lambda<int> getKeyProjection(Handle<Q13CustomerOrders> aggMe) override {
         return makeLambdaFromMember(aggMe, c_custkey);
    }

    Lambda<int> getValueProjection (Handle<Q13CustomerOrders> aggMe) override {
         return makeLambda(aggMe, [](Handle<Q13CustomerOrders>& aggMe) {
             return 1;
         });
    }


};


class Q13CustomerDistribution : public ClusterAggregateComp<Q13CountResult,
                                               Q13CountResult,
                                               int,
                                               int> {

public:

    ENABLE_DEEP_COPY


    Q13CustomerDistribution () {}

    Lambda<int> getKeyProjection(Handle<Q13CountResult> aggMe) override {
         return makeLambdaFromMember(aggMe, value);
    }

    Lambda<int> getValueProjection (Handle<Q13CountResult> aggMe) override {
         return makeLambda(aggMe, [](Handle<Q13CountResult>& aggMe) {
             return 1;
         });
    }


};

/*
    customer.join(order, $"c_custkey" === order("o_custkey")
      && !special(order("o_comment")), "left_outer")
      .groupBy($"o_custkey")
      .agg(count($"o_orderkey").as("c_count"))
      .groupBy($"c_count")
      .agg(count($"o_custkey").as("custdist"))
      .sort($"custdist".desc, $"c_count".desc)
*/

}

#endif
