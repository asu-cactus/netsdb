
#ifndef COMPUTE_PLAN_H
#define COMPUTE_PLAN_H

#include "Computation.h"
#include "PDBString.h"
#include "Object.h"
#include "LogicalPlan.h"
#include "PDBVector.h"
#include "Pipeline.h"
#include "ComputeInfo.h"
#include "SinkMerger.h"
#include "SinkShuffler.h"

// PRELOAD %ComputePlan%

namespace pdb {

typedef std::shared_ptr<LogicalPlan> LogicalPlanPtr;

// this is the basic type that is sent around a PDB cluster to store a computation that PDB is to
// execute
class ComputePlan : public Object {

    // this is the compiled plan
    String TCAPComputation;

    // this is the list of Computation objects that are going to be used to power the plan
    Vector<Handle<Computation>> allComputations;

    // this data structure contains both the compiled TCAP string, as well as an index of all of the
    // computations
    LogicalPlanPtr myPlan;

public:
    ENABLE_DEEP_COPY

    ComputePlan();

    ~ComputePlan() {
        myPlan = nullptr;
    }


    // constructor, takes as input the string to execute, as well as the vector of computations
    ComputePlan(String& TCAPComputation, Vector<Handle<Computation>>& allComputations);

    // this compiles the TCAPComputation string, returning a LogicalPlan object.  The resuting
    // object contains:
    //
    // (1) a graph of individual, SIMD-style operations.  This can be accessed via the
    // getComputations () method.
    // (2) a data structure containing all of the actual Computations that implement those
    // SIMD-style operations,
    //     as well as the Lambdas that are associated with each of those SIMD-style operations.
    //     Particular Computation
    //     objects can be accessed via the getNode () method (note that the argument to getNode ()
    //     is a string that
    //     names the computation; this string can be obtained via the getComputationName () method
    //     on the
    //     AtomicComputation objects stored in the graph of SIMD-style operations.
    //
    LogicalPlanPtr getPlan();

    // JiaNote: get producing computation name
    std::string getProducingComputationName(std::string sourceTupleSetName);

    // Note that once getPlan () has been called, ComputePlan object contains a C++ smart pointer
    // inside of it.
    // IT IS VERY DANGEROUS TO SEND SUCH A POINTER ACCROSS THE NETWORK.  Hence, after calling
    // getPlan () but before
    // this object is sent accross the network or written to disk, the following method MUST be
    // called to avoid
    // sending the smart pointer.
    void nullifyPlanPointer();

    // this builds a pipeline between the Computation that produces sourceTupleSetName and the
    // Computation
    // targetComputationName.  Since targetComputationName can have more than one input (in the case
    // of a join,
    // for example) the pipeline to targetComputationName is built on the link producig
    // targetTupleSetName.
    //
    // The lambda getPage is used by the pipeline to obtain new temp pages; it is assumed that a
    // page returned
    // by getPage will remain pinned until either discardTempPage or writeBackPage are called.  The
    // former is
    // called if the page can safely be destroyed because it has no useful data.  The latter is
    // called if the
    // page stores a pdb :: Object that contains the result of the computation.

    PipelinePtr buildPipeline(int threadId,
		              std::string sourceTupleSetName,
                              std::string targetTupleSetName,
                              std::string targetComputationName,
                              std::function<std::pair<void*, size_t>()> getPage,
                              std::function<void(void*)> discardTempPage,
                              std::function<void(void*)> writeBackPage,
                              std::map<std::string, ComputeInfoPtr>& params);


    PipelinePtr buildPipeline(int threadId,
		              std::string sourceTupleSetName,
                              std::string targetTupleSetName,
                              std::string targetComputationName,
                              std::function<std::pair<void*, size_t>()> getPage,
                              std::function<void(void*)> discardTempPage,
                              std::function<void(void*)> writeBackPage);


    // JiaNote: add new buildPipeline methods to avoid ambiguity
    PipelinePtr buildPipeline(int threadId,
		              std::vector<std::string> buildTheseTupleSets,
                              std::string sourceTupleSetName,
                              std::string targetComputationName,
                              std::function<std::pair<void*, size_t>()> getPage,
                              std::function<void(void*)> discardTempPage,
                              std::function<void(void*)> writeBackPage,
                              std::map<std::string, ComputeInfoPtr>& params);

    PipelinePtr buildPipeline(int threadId,
		              std::vector<std::string> buildTheseTupleSets,
                              std::string sourceTupleSetName,
                              std::string targetComputationName,
                              std::function<std::pair<void*, size_t>()> getPage,
                              std::function<void(void*)> discardTempPage,
                              std::function<void(void*)> writeBackPage);


    // JiaNote: add this to get sink merger
    SinkMergerPtr getMerger(std::string sourceTupleSetName,
                            std::string targetTupleSetName,
                            std::string targetComputationName);


    // JiaNote: add this to get partitioned sink merger
    SinkMergerPtr getPartitionedMerger(int partitionId,
                            std::string sourceTupleSetName,
                            std::string targetTupleSetName,
                            std::string targetComputationName);


    // JiaNote: add this to get sink shuffler
    SinkShufflerPtr getShuffler(std::string sourceTupleSetName,
                                std::string targetTupleSetName,
                                std::string targetComputationName);
};
}

#include "ComputePlan.cc"

#endif
