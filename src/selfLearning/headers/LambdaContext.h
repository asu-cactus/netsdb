#ifndef LAMBDA_CONTEXT_H
#define LAMBDA_CONTEXT_H

//class to describe a Lambda

#include "Object.h"
#include "PDBString.h"

namespace pdb {

class LambdaContext  {

public:
   
    LambdaContext () {}


    //constructor
    LambdaContext (long lambdaId, long jobId, std::string computationName, std::string lambdaName, int lambdaIndexInInputs) {

       this->lambdaId = lambdaId;
       this->jobId = jobId;
       this->computationName = computationName;
       this->lambdaName = lambdaName;
       this->lambdaIndexInInputs = lambdaIndexInInputs;

    }

    //typename
    std::string typeName;

    //identifier
    std::string identifier;

    //the id of the lambda
    long lambdaId;

    //the id of the Job in which the Lambda is executed
    long jobId;


    //the name of the Computation in which the Lambda is executed
    std::string computationName;

    //the name of the Lambda
    std::string lambdaName;

    //the index of lambda input in computation inputs
    int lambdaIndexInInputs;

    //distance to data
    double dataDistance = -1.0;

    //distance to join
    double joinDistance = -1.0;

    //distance to aggregation
    double aggDistance = -1.0;

    //frequency
    double frequency = -1.0;

    //selectivity
    double selectivity = -1.0;

    //number of distinct hash keys
    double avgNumHashKeys = -1.0;

    //count
    int count = 0;
};

}


#endif
