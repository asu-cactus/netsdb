#ifndef DOUBLE_SUM_RESULT_H
#define DOUBLE_SUM_RESULT_H


#include "Object.h"

// PRELOAD %DoubleSumResult%

namespace pdb {

class DoubleSumResult : public Object {

public:
    double total;
    int identifier;

    ENABLE_DEEP_COPY

    double getTotal() {
        return total;
    }

    int& getKey() {
        return identifier;
    }

    double& getValue() {
        return total;
    }
};
}


#endif
