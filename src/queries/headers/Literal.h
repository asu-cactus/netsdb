
#ifndef LITERAL_H
#define LITERAL_H

#include "LambdaType.h"

namespace pdb {

// this is used to store an actual, user-supplied function
template <typename Out>
class Literal : public LambdaType<Out> {

    std::vector<Handle<Object>*> params;
    std::function<Out()> myFunc;

public:
    std::string toString() {
        if (params.size() == 0) {
            return "function with no params";
        } else if (params.size() == 1) {
            std::string returnVal = "(function of param ";
            returnVal += std::to_string((size_t)params[0]) + ")";
            return returnVal;
        } else {
            std::string returnVal = "(function of params ";
            returnVal += std::to_string((size_t)params[0]);
            for (auto i : params) {
                returnVal += ", " + std::to_string((size_t)i);
            }
            returnVal += ")";
            return returnVal;
        }
    }

    ~Literal() {}

    void addParam(Handle<Object>* addMe) {
        params.push_back(addMe);
    }

    Literal(std::function<Out()> funcArg) {
        myFunc = funcArg;
    }

    FuncType getType() {
        return FuncType::UserDefined;
    }

    std::function<Out()> getFunc() {
        return myFunc;
    }

    std::vector<Handle<Object>*> getParams() {
        return params;
    }
};
}

#endif
