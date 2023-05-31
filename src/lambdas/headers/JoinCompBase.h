#ifndef JOINCOMPBASE_H
#define JOINCOMPBASE_H


namespace pdb {

class JoinCompBase : public Computation {

public:
    virtual ComputeExecutorPtr getExecutor(bool needToSwapAtts,
		                           int threadId,
                                           TupleSpec& hashedInputSchema,
                                           TupleSpec& pipelinedInputSchema,
                                           TupleSpec& pipelinedAttsToOperateOn,
                                           TupleSpec& pipelinedAttsToIncludeInOutput,
                                           ComputeInfoPtr arg) = 0;

    virtual ComputeExecutorPtr getExecutor(bool needToSwapAtts,
		                           int threadId,
                                           TupleSpec& hashedInputSchema,
                                           TupleSpec& pipelinedInputSchema,
                                           TupleSpec& pipelinedAttsToOperateOn,
                                           TupleSpec& pipelinedAttsToIncludeInOutput) = 0;
};
}

#endif
