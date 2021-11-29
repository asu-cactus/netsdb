#ifndef AVG_RESULT_H
#define AVG_RESULT_H

#include "Avg.h"
#include "Object.h"

// PRELOAD %AvgResult%

namespace pdb {

class AvgResult : public Object {

public:

    ENABLE_DEEP_COPY

    AvgResult () {}

    int key;

    Avg value;

    int & getKey() {
        return key;
    }

    Avg & getValue() {
        return value;
    }

    double getAvg() {
        return value.getAvg();
    }

};


}


#endif
