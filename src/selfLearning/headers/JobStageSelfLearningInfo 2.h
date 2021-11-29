#ifndef JOB_STAGE_SELF_LEARNING_INFO_H
#define JOB_STAGE_SELF_LEARNING_INFO_H

namespace pdb {


class JobStageSelfLearningInfo {

public:

    JobStageSelfLearningInfo (std::string sinkType, std::string targetComputationName, 
                              long jobId, long jobStageId, int stageId, int indexInInputs) {

         this->sinkType = sinkType;

         this->targetComputationName = targetComputationName;

         this->jobId = jobId;

         this->jobStageId = jobStageId;

         this->stageId = stageId;

         this->indexInInputs = indexInInputs;

    }

    void setIndexInInputs (int index) {
         this->indexInInputs = index;
    }

    std::string getSinkType() {
         return this->sinkType;
    }

    std::string getTargetComputationName() {
         return this->targetComputationName;
    }

    long getJobId() {
         return this->jobId;
    }

    long getJobStageId() {
         return this->jobStageId;
    }

    int getIndexInInputs() {
         return this->indexInInputs;
    }

    int getStageId() {
         return this->stageId;
    }

    /*double getSelectivity() {
         return this->selectivity;
    }*/

     
private:

    std::string sinkType;

    std::string targetComputationName;

    long jobId;

    long jobStageId;

    int  stageId;

    int indexInInputs;

    //double selectivity;


};

}


#endif
