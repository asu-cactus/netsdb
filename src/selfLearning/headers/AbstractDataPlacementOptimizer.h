#ifndef ABSTRACT_DATA_PLACEMENT_OPTIMIZER_H
#define ABSTRACT_DATA_PLACEMENT_OPTIMIZER_H


#include "LambdaIdentifier.h"



namespace pdb {

//defines common interfaces for various data placement optimizers

class AbstractDataPlacementOptimizer {

public:

//to return the optimal lambda for partitioning this set to be generated
virtual Handle<LambdaIdentifier> getBestLambda() = 0;

//to return the optimal page size for partitioning this set to be generated
virtual size_t getBestPageSize() = 0;

virtual size_t getEstimatedSize() = 0;

//to return the reward for DRL
virtual double getLastReward() { return 1.0; }

//to reduce producer job name
std::string getProducerJobName() {

    return producerJobName;

}

//to configure producer job name
void setProducerJobName (std::string producerJobName) {

    this->producerJobName = producerJobName;

}

protected:


//producer job name
std::string producerJobName;



};

}

#endif
