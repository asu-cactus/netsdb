
#ifndef JOIN_H
#define JOIN_H

#include "Handle.h"
#include "Lambda.h"
#include "Object.h"
#include "Query.h"

namespace pdb {

// this is the basic selection type... users derive from this class in order to write
// a selection query
template <typename Out, typename In1, typename In2, typename... Rest>
class Join : public Query<Out> {

    virtual Lambda<bool> getSelection(Handle<In1>& in1,
                                      Handle<In2>& in2,
                                      Handle<Rest>&... otherArgs) = 0;

    virtual Lambda<Handle<Out>> getProjection(Handle<In1>& in1,
                                              Handle<In2>& in2,
                                              Handle<Rest>&... otherArgs) = 0;

    // gets the number of intputs to this query type
    virtual int getNumInputs() override {
        return 2 + sizeof...(Rest);
    }

    // gets the name of the i^th input type...
    virtual std::string getIthInputType(int i) override {
        if (i == 0)
            return getTypeName<In1>();
        else if (i == 1)
            return getTypeName<In2>();
        else if (i >= getNumInputs())
            return "bad index";

        std::string nameArray[] = {getTypeName<Rest>()...};
        return nameArray[i - 2];
    }

    virtual std::string getQueryType() override {
        return "join";
    }
};
}

#endif
