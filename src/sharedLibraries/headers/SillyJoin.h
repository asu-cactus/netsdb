#ifndef SILLY_JOIN_H
#define SILLY_JOIN_H

// by Jia, Mar 2017

#include "JoinComp.h"
#include "PDBString.h"
#include "StringIntPair.h"
#include "LambdaCreationFunctions.h"


using namespace pdb;

class SillyJoin : public JoinComp<String, int, StringIntPair, String> {

public:
    ENABLE_DEEP_COPY

    SillyJoin() {}

    Lambda<bool> getSelection(Handle<int> in1,
                              Handle<StringIntPair> in2,
                              Handle<String> in3) override {
        // std :: cout << "SillyJoin selection: type code is " << in1.getExactTypeInfoValue() << ",
        // " << in2.getExactTypeInfoValue() << ", " << in3.getExactTypeInfoValue() << std :: endl;
        return (makeLambdaFromSelf(in1) == makeLambdaFromMember(in2, myInt)) &&
            (makeLambdaFromMember(in2, myString) == makeLambdaFromSelf(in3));
    }

    Lambda<Handle<String>> getProjection(Handle<int> in1,
                                         Handle<StringIntPair> in2,
                                         Handle<String> in3) override {
        // std :: cout << "SillyJoin projection: type code is " << in1.getExactTypeInfoValue() << ",
        // " << in2.getExactTypeInfoValue() << ", " << in3.getExactTypeInfoValue() << std :: endl;
        return makeLambdaFromMember(in2, myString);
    }
};


#endif
