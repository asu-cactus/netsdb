
#ifndef EXEC_COMPUTATION_H
#define EXEC_COMPUTATION_H

#include "Object.h"
#include "PDBString.h"

// PRELOAD %ExecuteComputation%

namespace pdb {

// encapsulates a request to run a query
class ExecuteComputation : public Object {

public:
    ExecuteComputation() {}
    ~ExecuteComputation() {}

    ExecuteComputation(std::string jobName, std::string tcapString, bool whetherToPreCompile=true) {
        this->jobName = jobName;
        this->tcapString = tcapString;
        this->whetherToPreCompile = whetherToPreCompile;
    }

    void setJobName(std::string jobName) {
        this->jobName = jobName;
    }

    std::string getJobName() {
        return this->jobName;
    }

    void setTCAPString(std::string tcapString) {
        this->tcapString = tcapString;
    }

    std::string getTCAPString() {
        return tcapString;
    }

    bool getWhetherToPreCompile() {
        return whetherToPreCompile;
    }

    ENABLE_DEEP_COPY

private:

    String jobName;
    String tcapString;
    bool whetherToPreCompile = true;    
};
}

#endif
