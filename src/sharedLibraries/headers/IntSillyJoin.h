#ifndef INT_SILLY_JOIN_H
#define INT_SILLY_JOIN_H

// by Jia, Mar 2017

#include "JoinComp.h"
#include "PDBString.h"
#include "StringIntPair.h"
#include "LambdaCreationFunctions.h"


using namespace pdb;

class IntSillyJoin : public JoinComp<int, int, StringIntPair, String> {

public:
    ENABLE_DEEP_COPY

    IntSillyJoin() {}

    Lambda<bool> getSelection(Handle<int> in1,
                              Handle<StringIntPair> in2,
                              Handle<String> in3) override {
        return (makeLambdaFromSelf(in1) == makeLambdaFromMember(in2, myInt)) &&
            (makeLambdaFromMember(in2, myString) == makeLambdaFromSelf(in3));
    }

    Lambda<Handle<int>> getProjection(Handle<int> in1,
                                      Handle<StringIntPair> in2,
                                      Handle<String> in3) override {
        return makeLambda(in1, [](Handle<int>& in1) { return in1; });
    }
};


#endif
