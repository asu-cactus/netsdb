
#ifndef LAMBDA_TYPE_H
#define LAMBDA_TYPE_H

#include <vector>
#include <functional>
#include <iostream>
#include <memory>

#include "FuncType.h"
#include "Handle.h"
#include "Object.h"
#include "Lambda.h"

namespace pdb {

// an instance of this class lives inside each of the different lambda types, and provides
// the basic functionality.  There a concre version of one of these associated with each lambda type
template <typename Out>
class LambdaType {

public:
    virtual std::string toString() = 0;
    virtual std::function<Out()> getFunc() = 0;
    virtual std::vector<Handle<Object>*> getParams() = 0;
    virtual void addParam(Handle<Object>* addMe) = 0;
    virtual FuncType getType() = 0;
    virtual ~LambdaType(){};
};
}

#endif
