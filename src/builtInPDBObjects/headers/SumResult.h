#ifndef SUM_RESULT_H
#define SUM_RESULT_H


#include "Object.h"

// PRELOAD %SumResult%

namespace pdb {

class SumResult : public Object {

public:
    int total;
    int identifier;

    ENABLE_DEEP_COPY

    int getTotal() {
        return total;
    }

    int& getKey() {
        return identifier;
    }

    int& getValue() {
        return total;
    }

};
}


#endif
