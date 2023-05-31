#ifndef ABSTRACT_JOBSTAGE_H
#define ABSTRACT_JOBSTAGE_H

#include "../../conf/headers/DataTypes.h"

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

    bool getMaterializeOutput() {
        return this->materializeOutput;
    }

    void setMaterializeOutput(bool materializeOutput) {
        this->materializeOutput = materializeOutput;
    }


    bool getSourceMaterialized() {
        return this->sourceMaterialized;
    }

    void setSourceMaterialized(bool sourceMaterialized) {
        this->sourceMaterialized = sourceMaterialized;
    }

    bool getHashMaterialized() {
        return this->hashMaterialized;
    }

    void setHashMaterialized(bool hashMaterialized) {
        this->hashMaterialized = hashMaterialized;
    }



protected:
    String jobId;

    //Does the output of this stage have to be materialized
    bool materializeOutput = false;

    //Is the source materialized
    bool sourceMaterialized = false;

    //Is the hashtable materialized
    bool hashMaterialized = false;

};
}

#endif
