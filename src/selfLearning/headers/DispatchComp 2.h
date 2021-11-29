
#ifndef DISPATCH_COMP
#define DISPATCH_COMP

#include "Computation.h"


namespace pdb {

// this dispatches items of type InputClass.  To dispatch an item, the result of getKeyProjection
// () is
// used to extract a key from on input, and the key is used for partitioning
// Note that keys must have hash () defined.
//
template <class InputClass, class KeyClass>
class DispatchComp : public Computation {

    // gets the operation tht extracts a key from an input object
    virtual Lambda<KeyClass> getKeyProjection(Handle<InputClass> aggMe) = 0;

    // extract the key projection
    void extractLambdas(std::map<std::string, GenericLambdaObjectPtr>& returnVal) override {
        int suffix = 0;
        Handle<InputClass> checkMe = nullptr;
        Lambda<KeyClass> keyLambda = getKeyProjection(checkMe);
        keyLambda.toMap(returnVal, suffix);
    }

    // this is an dispatch comp
    std::string getComputationType() override {
        return std::string("DispatchComp");
    }

};
}

#endif
