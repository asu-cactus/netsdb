#ifndef SIMPLE_MOVIE_WRITE_H
#define SIMPLE_MOVIE_WRITE_H

// by Sourav, Jun 2017

//#include "VectorSink.h"
//#include "ComputePlan.h"
//#include "TupleSpec.h"
#include "Handle.h"
#include "Lambda.h"
#include "Supervisor.h"
#include "Employee.h"
#include "LambdaCreationFunctions.h"
#include "UseTemporaryAllocationBlock.h"
#include "Pipeline.h"
#include "SetWriter.h"
#include "SelectionComp.h"
#include "AggregateComp.h"
#include "JoinComp.h"
#include "ScanSet.h"
#include "VectorSink.h"
#include "HashSink.h"
#include "MapTupleSetIterator.h"
#include "VectorTupleSetIterator.h"
#include "ComputePlan.h"
#include "StringIntPair.h"
#include "QueryGraphAnalyzer.h"

#include "MovieStar.h"
#include "StarsIn.h"

using namespace pdb;

class SimpleMovieWrite : public SetWriter<MovieStar> {

public:
    ENABLE_DEEP_COPY

    // eventually, this method should be moved into a class that works with the system to
    // iterate through pages that are pulled from disk/RAM by the system... a programmer
    // should not provide this particular method
    ComputeSinkPtr getComputeSink(TupleSpec& consumeMe,
                                  TupleSpec& whichAttsToOpOn,
                                  TupleSpec& projection,
                                  ComputePlan& plan) override {
        return std::make_shared<VectorSink<MovieStar>>(consumeMe, projection);
    }
};

#endif
