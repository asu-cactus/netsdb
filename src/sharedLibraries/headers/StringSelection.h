
#ifndef STRING_SELECT_CC
#define STRING_SELECT_CC

#include "Selection.h"
#include "PDBVector.h"
#include "GetVTable.h"
#include "PDBString.h"

using namespace pdb;
class StringSelection : public Selection<String, String> {

public:
    ENABLE_DEEP_COPY

    SimpleLambda<bool> getSelection(Handle<String>& checkMe) override {
        return makeSimpleLambda(checkMe, [&]() {
            return (*(checkMe) == "Joe Johnson488") || (*(checkMe) == "Joe Johnson489");
        });
    }

    SimpleLambda<bool> getProjectionSelection(Handle<String>& checkMe) override {
        return makeSimpleLambda(checkMe, [&]() {
            return (*(checkMe) == "Joe Johnson488") || (*(checkMe) == "Joe Johnson489");
        });
    }


    SimpleLambda<Handle<String>> getProjection(Handle<String>& checkMe) override {
        return makeSimpleLambda(checkMe, [&]() { return checkMe; });
    }
};


#endif
