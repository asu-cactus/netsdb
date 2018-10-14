#ifndef JOBSTAGE_CONTEXT_H
#define JOBSTAGE_CONTEXT_H

//class to describe a JobStage


namespace pdb {

class JobStageContext  {

public:


//the ID of the job instance
long jobInstanceId;

//the ID of the job stage
long jobStageId;

//the ID of the source data
long dataId;

//the size of the source data
size_t sourceSize;

//the stageId of the job stage in the job
int stageId;

//the size of the sink data
size_t sinkSize;

//constructor
JobStageContext (long jobInstanceId, long jobStageId, long dataId, size_t sourceSize, int stageId, size_t sinkSize) {

    this->jobInstanceId = jobInstanceId;
    this->jobStageId = jobStageId;
    this->dataId = dataId;
    this->sourceSize = sourceSize;
    this->stageId = stageId;
    this->sinkSize = sinkSize;

}



};

}



#endif
