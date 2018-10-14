
#ifndef STRINGINTPAIR_MULTI_SELECT_H
#define STRINGINTPAIR_MULTI_SELECT_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "MultiSelectionComp.h"
#include "Employee.h"
#include "PDBVector.h"
#include "PDBString.h"
#include "StringIntPair.h"

using namespace pdb;
class StringIntPairMultiSelection : public MultiSelectionComp<StringIntPair, StringIntPair> {

public:
    ENABLE_DEEP_COPY

    StringIntPairMultiSelection() {}

    Lambda<bool> getSelection(Handle<StringIntPair> checkMe) override {
        return makeLambda(checkMe,
                          [](Handle<StringIntPair>& checkMe) { return checkMe->myInt % 9 == 0; });
    }

    Lambda<Vector<Handle<StringIntPair>>> getProjection(Handle<StringIntPair> checkMe) override {
        return makeLambda(checkMe, [](Handle<StringIntPair>& checkMe) {
            Vector<Handle<StringIntPair>> myVec;
            for (int i = 0; i < 10; i++) {
                Handle<StringIntPair> myPair =
                    makeObject<StringIntPair>("Hi", (checkMe->myInt) * i);
                myVec.push_back(myPair);
            }
            return myVec;
        });
    }
};


#endif
