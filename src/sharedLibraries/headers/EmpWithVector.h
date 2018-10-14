#ifndef EMP_WITH_VEC_H
#define EMP_WITH_VEC_H

#include "Supervisor.h"

using namespace pdb;

class EmpWithVector : public Object {

    Supervisor myDude;
    Vector<double> myVec;

public:
    ENABLE_DEEP_COPY

    EmpWithVector() {}

    EmpWithVector(Supervisor& useMe, Vector<double>& andMe) : myDude(useMe), myVec(andMe) {}

    Supervisor& getEmp() {
        return myDude;
    }

    Vector<double>& getVector() {
        return myVec;
    }
};


#endif
