#ifndef STRING_SELECTION_OF_STRING_INT_PAIR_H
#define STRING_SELECTION_OF_STRING_INT_PAIR_H

// by Jia, May 2017

#include "SelectionComp.h"
#include "StringIntPair.h"
#include "LambdaCreationFunctions.h"

using namespace pdb;

class StringSelectionOfStringIntPair : public SelectionComp<String, StringIntPair> {

public:
    ENABLE_DEEP_COPY

    StringSelectionOfStringIntPair() {}

    Lambda<bool> getSelection(Handle<StringIntPair> checkMe) override {
        return makeLambda(checkMe, [](Handle<StringIntPair>& checkMe) {
            if (((*checkMe).myInt % 3 == 0) && ((*checkMe).myInt < 1000)) {
                return true;
            } else {
                return false;
            }
        });
    }

    Lambda<Handle<String>> getProjection(Handle<StringIntPair> checkMe) override {
        return makeLambdaFromMember(checkMe, myString);
    }
};

#endif
