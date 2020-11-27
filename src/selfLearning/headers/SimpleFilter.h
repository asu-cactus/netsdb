#ifndef SIMPLE_FILTER_H
#define SIMPLE_FILTER_H


#include <memory>
#include "Handle.h"
#include "PDBVector.h"
#include "PDBMap.h"
#include "DataTypes.h"

namespace pdb {


class SimpleFilter;
typedef std::shared_ptr<SimpleFilter> SimpleFilterPtr;

class SimpleFilter {

private:


    // this is a lambda that we'll call to process input
    std::function<bool(Handle<Object> &)> processInput;

public:

    // constructor
    SimpleFilter(std::function<bool(Handle<Object> &)> processInput) {

         this->processInput = processInput;
    }

    std::function<bool(Handle<Object> &)>& getFilter() {
         return this->processInput;
    }

};

}

#endif

