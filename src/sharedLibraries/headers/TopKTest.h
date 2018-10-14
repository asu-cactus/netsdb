#ifndef TOP_K_TEST_H
#define TOP_K_TEST_H

#include "TopKComp.h"
#include "EmpWithVector.h"

using namespace pdb;

class TopKTest : public TopKComp<EmpWithVector, double, Handle<EmpWithVector>> {

    // this is the query vector that we are computing the distance to
    Vector<double> query;

    // and the size of the result
    unsigned k = 0;

public:
    ENABLE_DEEP_COPY

    TopKTest() {}

    TopKTest(Vector<double>& myQuery, unsigned k) : query(myQuery), k(k) {}

    // this just looks at the vector contained in the EmpWithVector object, and computes the
    // distance to the query vector as an L2 norm
    double getScore(Handle<EmpWithVector>& forMe) {

        // we get a c-style pointer to the array so that we don't have to repeatedly call
        // the dereference operator ->, which can be slow
        double* queryArray = query.c_ptr();
        double* inArray = forMe->getVector().c_ptr();
        unsigned sz = query.size();
        double distance = 0;
        for (int i = 0; i < sz; i++) {
            distance -= (queryArray[i] - inArray[i]) * (queryArray[i] - inArray[i]);
        }
        return distance;
    }

    // here we simply create a TopKQueue with the given queue size, and add the pair (getScore
    // (aggMe), aggMe)
    Lambda<TopKQueue<double, Handle<EmpWithVector>>> getValueProjection(
        Handle<EmpWithVector> aggMe) override {
        return makeLambda(aggMe, [&](Handle<EmpWithVector>& aggMe) {
            return TopKQueue<double, Handle<EmpWithVector>>(k, getScore(aggMe), aggMe);
        });
    }
};


#endif
