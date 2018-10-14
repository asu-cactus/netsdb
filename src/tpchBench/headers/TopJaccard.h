#ifndef TOP_JACCARD_H
#define TOP_JACCARD_H

#include "Object.h"
#include "PDBVector.h"
#include "PDBString.h"
#include "Handle.h"
#include "TopKComp.h"
#include "Customer.h"
#include "AllParts.h"

namespace pdb {

// For each customer, constructs a list of unique part IDs, and then checks to compute the
// Jaccard similarity to the query part list.  It then returns the k part lists that are most
// simiar to the query part list
class TopJaccard : public TopKComp<Customer, double, Handle<AllParts>> {

    Vector<int> partsToCheckFor;
    int k = 10;

public:
    ENABLE_DEEP_COPY

    TopJaccard() {}

    TopJaccard(int kIn, Vector<int> partsToCheckForIn)
        : k(kIn), partsToCheckFor(partsToCheckForIn) {}

    Lambda<TopKQueue<double, Handle<AllParts>>> getValueProjection(
        Handle<Customer> aggMe) override {
        return makeLambda(aggMe, [&](Handle<Customer>& aggMe) {
            Handle<AllParts> myGuy = makeObject<AllParts>();
            return TopKQueue<double, Handle<AllParts>>(
                k, myGuy->fill(partsToCheckFor.c_ptr(), partsToCheckFor.size(), *aggMe), myGuy);
        });
    }
};
}
#endif
