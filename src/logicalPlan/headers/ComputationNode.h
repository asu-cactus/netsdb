
#ifndef COMPUTATION_NODE_H
#define COMPUTATION_NODE_H

#include "Computation.h"

namespace pdb {

// this stores all of the information extracted about a node in the computation plan, including
// (a) all of the lambdas extracted from the node, and (b) a reference to the actual computation
class ComputationNode {

private:
    // allows us to access the executors for this plan node... the key is a name of a lambda
    std::map<std::string, GenericLambdaObjectPtr> allLambdas;

    // the computation itself
    Handle<Computation> me;

public:
    ComputationNode() {
        me = nullptr;
    }

    ComputationNode(const ComputationNode& toMe) {
        allLambdas = toMe.allLambdas;
        me = toMe.me;
    }

    ComputationNode& operator=(const ComputationNode& toMe) {
        allLambdas = toMe.allLambdas;
        me = toMe.me;
        return *this;
    }

    // simple constructor... extracts the set of lambdas from this compuation
    ComputationNode(Handle<Computation>& me) : me(me) {
        me->extractLambdas(allLambdas);
    }

    Computation& getComputation() {
        return *me;
    }

    // JiaNote: add a new method to get Handle<Computation> for unsafeCast
    Handle<Computation> getComputationHandle() {
        return me;
    }

    // get the particular lambda
    GenericLambdaObjectPtr getLambda(std::string me) {
        if (allLambdas.count(me) == 0) {
            std::cout << "This is bad.  Didn't find a lambda corresponding to " << me << "\n";
            exit(1);
        }
        return allLambdas[me];
    }
};
}

#endif
