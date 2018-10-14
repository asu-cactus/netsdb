
#ifndef LAMBDA_IDENTIFIER_H
#define LAMBDA_IDENTIFIER_H


#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

// PRELOAD %LambdaIdentifier%

namespace pdb {

// encapsulates a Lambda that can be fetched from SelfLearning database
class LambdaIdentifier : public Object {

public:

    ENABLE_DEEP_COPY

    LambdaIdentifier() {}
    ~LambdaIdentifier() {}

     LambdaIdentifier(std::string jobName, std::string computationName, std::string lambdaName) {
        this->jobName = jobName;
        this->computationName = computationName;
        this->lambdaName = lambdaName;
    }



    LambdaIdentifier(long lambdaId, 
                     std::string lambdaIdentifier, 
                     std::string lambdaInputClass, 
                     std::string jobName, 
                     std::string computationName, 
                     std::string lambdaName) {
        this->lambdaId = lambdaId;
        this->lambdaIdentifier = lambdaIdentifier;
        this->lambdaInputClass = lambdaInputClass;
        this->jobName = jobName;
        this->computationName = computationName;
        this->lambdaName = lambdaName;
    }

    long getLambdaId() {
        return this->lambdaId;
    }

    std::string getLambdaIdentifier() {
        return this->lambdaIdentifier;
    }

    std::string getLambdaInputClass() {
        return this->lambdaInputClass;
    }

    std::string getJobName() {
        return this->jobName;
    }

    std::string getComputationName() {
        return this->computationName;
    }

    std::string getLambdaName() {
        return this->lambdaName;
    }

    void setLambdaId(long lambdaId) {
        this->lambdaId = lambdaId;
    }

    void setLambdaIdentifier(std::string lambdaIdentifier) {
        this->lambdaIdentifier = lambdaIdentifier;
    }

    void setLambdaInputClass(std::string lambdaInputClass) {
        this->lambdaInputClass = lambdaInputClass;
    }

    void setJobName(std::string jobName) {
        this->jobName = jobName;
    }

    void setComputationName(std::string computationName) {
        this->computationName = computationName;
    }

    void setLambdaName(std::string lambdaName) {
        this->lambdaName = lambdaName;
    }

private:
    long lambdaId;
    std::string lambdaIdentifier;
    std::string lambdaInputClass;
    std::string jobName;
    std::string computationName;
    std::string lambdaName;
};
}

#endif
