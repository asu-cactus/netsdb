#ifndef JOB_INSTANCE_CONTEXT_H
#define JOB_INSTANCE_CONTEXT_H


//class to describe a Job instance, which is one execution of a Job


namespace pdb {

class JobInstanceContext {

public:

//constructor
JobInstanceContext (long jobId, long jobInstanceId, size_t inputSize, double initialLatency, long initialJobInstanceId, long submitTime,  long endTime) {

    this->jobId = jobId;
    this->jobInstanceId = jobInstanceId;
    this->inputSize = inputSize;
    this->initialLatency = initialLatency;
    this->initialJobInstanceId = initialJobInstanceId;
    this->submitTime = submitTime;
    this->endTime = endTime;

}

long jobId;

long jobInstanceId;

size_t inputSize;

double initialLatency;

long initialJobInstanceId;

long submitTime;

long endTime;

};


}
#endif
