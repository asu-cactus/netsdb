#ifndef CARTESIAN_JOIN_H
#define CARTESIAN_JOIN_H

// by Jia, Mar 2017

#include "JoinComp.h"
#include "PDBString.h"
#include "StringIntPair.h"
#include "LambdaCreationFunctions.h"


using namespace pdb;

class CartesianJoin : public JoinComp<StringIntPair, int, String> {

public:
    ENABLE_DEEP_COPY

    CartesianJoin() {}

    Lambda<bool> getSelection(Handle<int> in1, Handle<String> in2) override {
        std::cout << "CartesianJoin selection: type code is " << in1.getExactTypeInfoValue() << ", "
                  << in2.getExactTypeInfoValue() << std::endl;
        return makeLambda(in1, in2, [](Handle<int>& in1, Handle<String>& in2) { return true; });
    }

    Lambda<Handle<StringIntPair>> getProjection(Handle<int> in1, Handle<String> in2) override {
        std::cout << "CartesianJoin projection: type code is " << in1.getExactTypeInfoValue()
                  << ", " << in2.getExactTypeInfoValue() << std::endl;
        return makeLambda(in1, in2, [](Handle<int>& in1, Handle<String>& in2) {
            Handle<StringIntPair> pair = makeObject<StringIntPair>((*in2), (*in1));
            return pair;
        });
    }
};


#endif
