//
// Created by barnett on 8/26/16.
//

#ifndef QUERYALGO_H
#define QUERYALGO_H


#include "Object.h"

namespace pdb {
class QueryBase;

class QueryAlgo : public Object {
public:
    virtual void forSelection(QueryBase& selection) = 0;

    virtual void forSet(QueryBase& set) = 0;

    virtual void forQueryOutput(QueryBase& output) = 0;
};
}

#endif  // QUERYALGO_H
