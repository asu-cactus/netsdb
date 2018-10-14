#ifndef INT_SELECTION_OF_STRING_INT_PAIR_H
#define INT_SELECTION_OF_STRING_INT_PAIR_H

// by Jia, May 2017

#include "SelectionComp.h"
#include "StringIntPair.h"
#include "LambdaCreationFunctions.h"

using namespace pdb;

class IntSelectionOfStringIntPair : public SelectionComp<int, StringIntPair> {

public:
    ENABLE_DEEP_COPY

    IntSelectionOfStringIntPair() {}

    Lambda<bool> getSelection(Handle<StringIntPair> checkMe) override {
        return makeLambda(checkMe, [](Handle<StringIntPair>& checkMe) {
            if (((*checkMe).myInt % 3 == 0) && ((*checkMe).myInt < 1000)) {
                return true;
            } else {
                return false;
            }
        });
    }

    Lambda<Handle<int>> getProjection(Handle<StringIntPair> checkMe) override {
        return makeLambda(checkMe, [](Handle<StringIntPair>& checkMe) {
            Handle<int> ret = makeObject<int>((*checkMe).myInt);
            return ret;
        });
    }
};

#endif
