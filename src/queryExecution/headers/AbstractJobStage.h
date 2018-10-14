#ifndef ABSTRACT_JOBSTAGE_H
#define ABSTRACT_JOBSTAGE_H


namespace pdb {

//this class encapsulates the common interface for JobStages
class AbstractJobStage : public Object {

public:
    void setJobId(std::string jobId) {
        this->jobId = jobId;
    }

    std::string getJobId() {
        return this->jobId;
    }


    virtual std::string getJobStageType() = 0;
    virtual JobStageID getStageId() = 0;
    virtual void print() = 0;

protected:
    String jobId;
};
}

#endif
