#ifndef QUERY03_H
#define QUERY03_H


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

class Q03CustomerSelection : public SelectionComp<Customer, Customer> {

private:

   String segment;

public:

    ENABLE_DEEP_COPY

    Q03CustomerSelection () {}

    Q03CustomerSelection (std::string segment) {
        this->segment = segment;
    }

    Lambda<bool> getSelection(Handle<Customer> checkMe) override {
        return makeLambda(checkMe,
           [&](Handle<Customer>& checkMe) { 
               if (checkMe->c_mktsegment == this->segment) {
                   return true;
               } else {
                   return false;
               } 
           });
    }

    Lambda<Handle<Customer>> getProjection(Handle<Customer> checkMe) override {
        return makeLambda(checkMe, [](Handle<Customer>& checkMe) { 
            return checkMe; 
        });
    }


};


class Q03OrderSelection : public SelectionComp<Order, Order> {

private:

   String date;

public:

    ENABLE_DEEP_COPY

    Q03OrderSelection () {}

    Q03OrderSelection (std::string date) {
        this->date = date;
    }

    Lambda<bool> getSelection(Handle<Order> checkMe) override {
        return makeLambda(checkMe,
           [&](Handle<Order>& checkMe) { return (strcmp(checkMe->o_orderdate.c_str(), this->date.c_str()) < 0); });
    }

    Lambda<Handle<Order>> getProjection(Handle<Order> checkMe) override {
        return makeLambda(checkMe, [](Handle<Order>& checkMe) { 
                 return checkMe; 
        });
    }


};


class Q03LineItemSelection : public SelectionComp<LineItem, LineItem> {

private:

   String date;

public:

    ENABLE_DEEP_COPY

    Q03LineItemSelection () {}

    Q03LineItemSelection (std::string date) {
        this->date = date;
    }

    Lambda<bool> getSelection(Handle<LineItem> checkMe) override {
        return makeLambda(checkMe,
           [&](Handle<LineItem>& checkMe) { return (strcmp(checkMe->l_shipdate.c_str(), this->date.c_str()) > 0); });
    }

    Lambda<Handle<LineItem>> getProjection(Handle<LineItem> checkMe) override {
        return makeLambda(checkMe, [](Handle<LineItem>& checkMe) { 
           return checkMe; 
        });
    }


};


class Q03JoinOut : public Object {

public:

    ENABLE_DEEP_COPY

    int l_orderkey;
    double revenue;
    String o_orderdate;
    int o_shippriority;

    Q03JoinOut () {}

    Q03JoinOut (LineItem& lineitem, Order& order) {
        this->l_orderkey = lineitem.l_orderkey;
        this->o_orderdate = order.o_orderdate;
        this->o_shippriority = order.o_shippriority;
        this->revenue = lineitem.l_extendedprice * (1 - lineitem.l_discount);
    }    


};

class Q03Join : public JoinComp<Q03JoinOut, Customer, Order, LineItem> {

public:

    ENABLE_DEEP_COPY

    Q03Join () {}

    Lambda<bool> getSelection (Handle<Customer> in1, Handle<Order> in2, Handle<LineItem> in3) override {
        return makeLambdaFromMember(in1, c_custkey) == makeLambdaFromMember(in2,
           o_custkey) && makeLambdaFromMember(in2, o_orderkey) == makeLambdaFromMember(in3,
           l_orderkey);
         
    }
    
    Lambda<Handle<Q03JoinOut>> getProjection(Handle<Customer> in1, Handle<Order> in2, Handle<LineItem> in3) override {
        return makeLambda(in1, in2, in3, [](Handle<Customer>& in1, Handle<Order>& in2, Handle<LineItem>& in3) {  
             Handle<Q03JoinOut> ret = makeObject<Q03JoinOut>(*in3, *in2);
             return ret;
         });
    }


};


class Q03KeyClass : public Object {

public:

    ENABLE_DEEP_COPY

    int l_orderkey;
    String o_orderdate;
    int o_shippriority;

    Q03KeyClass () {}

    Q03KeyClass (Q03JoinOut & joinOut) {
        this->l_orderkey = joinOut.l_orderkey;
        this->o_orderdate = joinOut.o_orderdate;
        this->o_shippriority = joinOut.o_shippriority;
    }

    size_t hash() const {
        return l_orderkey;
    }

    bool operator==(const Q03KeyClass& toMe) {
        if ((l_orderkey == toMe.l_orderkey) && (o_orderdate == toMe.o_orderdate)
              &&(o_shippriority == toMe.o_shippriority)) {
            return true;
        } else {
            return false;
        }
    }

};


class Q03AggOut : public Object {

public:

    ENABLE_DEEP_COPY

    Q03KeyClass key;
    double value;

    Q03AggOut () {}

    Q03KeyClass & getKey() {
        return key;
    }

    double & getValue() {
        return value;
    }

};


class Q03Agg : public ClusterAggregateComp<Q03AggOut,
                                               Q03JoinOut,
                                               Q03KeyClass,
                                               double> {

public:

    ENABLE_DEEP_COPY


    Q03Agg () {}

    Lambda<Q03KeyClass> getKeyProjection(Handle<Q03JoinOut> aggMe) override {
         return makeLambda(aggMe, [](Handle<Q03JoinOut>& checkMe) { 
             Q03KeyClass ret(*checkMe);
             return ret;
         });
    }

    Lambda<double> getValueProjection (Handle<Q03JoinOut> aggMe) override {
         return makeLambdaFromMember(aggMe, revenue);
    }


};



}

#endif
