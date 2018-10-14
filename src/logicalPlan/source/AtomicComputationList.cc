
#ifndef COMP_LIST_CC
#define COMP_LIST_CC

#include "AtomicComputationList.h"
#include "AtomicComputationClasses.h"
#include "PDBDebug.h"

// gets the computation that builds the tuple set with the specified name
AtomicComputationPtr AtomicComputationList::getProducingAtomicComputation(std::string outputName) {
    std::cout << "outputName is " << outputName << std::endl;
    if (producers.count(outputName) == 0) {
        std::cout << "This could be bad... can't find the guy producing output " << outputName
                 << ".\n";
        for (auto iter = producers.begin(); iter!=producers.end(); ++iter) {
             std::cout << "producers[" << iter->first << "] exists" << std::endl;
        }
    }
    return producers[outputName];
}

// gets the list of comptuations that consume the tuple set with the specified name
std::vector<AtomicComputationPtr>& AtomicComputationList::getConsumingAtomicComputations(
    std::string inputName) {
    if (consumers.count(inputName) == 0) {
        std::cout << "This could be bad... can't find the guy consuming input " << inputName
                 << ".\n";
    }
    return consumers[inputName];
}

// this effectively gets all of the leaves of the graph, since it returns all of the scans... every
// AtomicComputationPtr in the returned list will point to a ScanSet object
std::vector<AtomicComputationPtr>& AtomicComputationList::getAllScanSets() {
    return scans;
}

// add an atomic computation to the graph
void AtomicComputationList::addAtomicComputation(AtomicComputationPtr addMe) {

    if (addMe->getAtomicComputationType() == "Scan") {
        scans.push_back(addMe);
    }

    producers[addMe->getOutputName()] = addMe;
    if (consumers.count(addMe->getInputName()) == 0) {
        std::vector<AtomicComputationPtr> rhs;
        consumers[addMe->getInputName()] = rhs;
    }
    consumers[addMe->getInputName()].push_back(addMe);

    // now, see if this guy is a join; join is special, because we have to add both inputs to the
    // join to the consumers map
    if (addMe->getAtomicComputationType() == "JoinSets") {
        ApplyJoin* myPtr = (ApplyJoin*)addMe.get();
        consumers[myPtr->getRightInput().getSetName()].push_back(addMe);
    }

    // kill the copy of the shared pointer that is inside him
    addMe->destroyPtr();
}

std::ostream& operator<<(std::ostream& os, const AtomicComputationList& printMe) {
    for (auto& a : printMe.producers) {
        os << a.second->output << " <= " << a.second->getAtomicComputationType() << "("
           << a.second->input << ", " << a.second->projection;
        os << ")\n";
    }
    return os;
}

#endif
