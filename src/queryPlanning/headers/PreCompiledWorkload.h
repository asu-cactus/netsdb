#ifndef PRECOMPILED_WORKLOAD_H
#define PRECOMPILED_WORKLOAD_H

#include "DataTypes.h"
#include "InterfaceFunctions.h"
#include "AbstractJobStage.h"
#include "TupleSetJobStage.h"
#include "AggregationJobStage.h"
#include "BroadcastJoinBuildHTJobStage.h"
#include "HashPartitionedJoinBuildHTJobStage.h"
#include "SetIdentifier.h"

namespace pdb {


// create a smart pointer for PreCompiledWorkload objects
class PreCompiledWorkload;
typedef shared_ptr<PreCompiledWorkload> PreCompiledWorkloadPtr;



// this class wraps a set of executable job stages that are pre-compiled for a workload

class PreCompiledWorkload {

private:
  // a set of job stages
  std::vector<Handle<AbstractJobStage>> stages;

  // intermediate sets
  std::vector<Handle<SetIdentifier>> intermediateSets;



public:

  //return stages
  std::vector<Handle<AbstractJobStage>> & getStages() {

      return stages;

  }

  //return intermediate sets
  std::vector<Handle<SetIdentifier>> & getIntermediateSets() {

      return intermediateSets;

  }

  //print all stages and intermediate sets
  void print() {

     std::cout << "this workload has " << stages.size() << " materialized stages" << std::endl;
     std::cout << "this workload has " << intermediateSets.size() << " intermediate sets" << std::endl;

     for (int i = 0; i < stages.size(); i++) {
         std::cout << "to print the " << i << "-th stage" << std::endl;
         stages[i]->print();

     }

     for (int i = 0; i < intermediateSets.size(); i++) {

         std::cout << "to print the " << i << "-th set" << std::endl;
         intermediateSets[i]->print();

     }

  }

  // constructor
  PreCompiledWorkload( std::vector<Handle<AbstractJobStage>> inputStages,
              std::vector<Handle<SetIdentifier>> inputIntermediateSets) {


      makeObjectAllocatorBlock(25 * 1024 * 1024, true);

      for (int i = 0; i < inputStages.size(); i++) {
           Handle<AbstractJobStage> curStage = inputStages[i];
           curStage->print();
           std::string stageType = curStage->getJobStageType();
           Handle<AbstractJobStage> copiedStage = deepCopyToCurrentAllocationBlock<AbstractJobStage>(curStage);
	   copiedStage->print();
           stages.push_back(copiedStage);
      }

      for (int i = 0; i < inputIntermediateSets.size(); i++) {
          Handle<SetIdentifier> copiedSet = deepCopyToCurrentAllocationBlock<SetIdentifier> (inputIntermediateSets[i]);
          copiedSet->print();
          intermediateSets.push_back(copiedSet);
      }

  }


  

  // destructor
  ~PreCompiledWorkload() {}

};
}
#endif
