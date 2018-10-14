#ifndef METHOD_JOIN_H
#define METHOD_JOIN_H

// by Jia, Mar 2017

#include "JoinComp.h"
#include "PDBString.h"
#include "StringIntPair.h"
#include "LambdaCreationFunctions.h"


using namespace pdb;

class MethodJoin : public JoinComp<StringIntPair, StringIntPair, StringIntPair> {

public:
    ENABLE_DEEP_COPY

    MethodJoin() {}

    Lambda<bool> getSelection(Handle<StringIntPair> in1, Handle<StringIntPair> in2) override {
        return (makeLambdaFromMethod(in1, getSillyInt) == makeLambdaFromMethod(in2, getSillyInt)) &&
            (makeLambdaFromMethod(in1, getSillyString) ==
             makeLambdaFromMethod(in2, getSillyString));
    }

    Lambda<Handle<StringIntPair>> getProjection(Handle<StringIntPair> in1,
                                                Handle<StringIntPair> in2) override {
        return makeLambda(
            in1, in2, [](Handle<StringIntPair>& in1, Handle<StringIntPair>& in2) { return in2; });
    }
};


#endif
