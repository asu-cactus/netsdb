#ifndef AVG_H
#define AVG_H


#include "Object.h"

// PRELOAD %Avg%

namespace pdb {

class Avg : public Object {

public:
    double total;
    int count;

    ENABLE_DEEP_COPY


    Avg () {}

    Avg (double total, int count) {
        this->total = total;
        this->count = count;
    }

    double getAvg() {
        return total/(double)(count);
    }

    Avg & operator+ (Avg& avg) {
        this->total += avg.total;
        this->count += avg.count;
        return *this;
    }
};
}


#endif
