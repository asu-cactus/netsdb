#ifndef JOB_CONTEXT_H
#define JOB_CONTEXT_H


//Class to describe a Job
//Note that a job may have many instances, and each instance is one execution of the job
//We assume all instances of the same job share the common JobName, but each instance has its own JobID


#include "Object.h"
#include "PDBString.h"
#include "ComputePlan.h"
#include "Handle.h"
#include "Computation.h"
#include "DispatchComp.h"
#include <time.h>

namespace pdb {

class JobContext : public Object {

public:

//constructor for Query job
JobContext (std::string jobType, std::string jobName, std::string tcap, Handle<Vector<Handle<Computation>>> allComputations) {

    this->jobType = jobType;
    this->jobName = jobName;
    this->tcap = tcap;
    this->allComputations = allComputations;
    this->dispatchComputation = nullptr
}

//constructor for DispatchData job
JobContext (std::string jobType, std::string jobName, Handle<DispatchComp> dispatchComputation) {

    this->jobType = jobType;
    this->jobName = jobName;
    this->dispatchComputation = dispatchComputation;
    this->tcap = "";
    this->allComputations = nullptr;

}

//getters/setters

void setJobType (std::string jobType) {

    this->jobType = jobType;

}

std::string getJobType() {

    return this->jobType;

}



void setJobName (std::string jobName) {

    this->jobName = jobName;

}

std::string getJobName() {

    return this->jobName;

}


void setTCAP (std::string tcap) {

    this->tcap = tcap;

}

std::string getTCAP () {

    return this->tcap;

}

void setComputations (Handle<Vector<Handle<Computation>>> allComputations) {

    this->allComputations = allComputations;

}

Handle<Vector<Handle<Computation>>> getComputations() {

    return this->allComputations;

}

void setDispatchComputation (Handle<DispatchComp> dispatchComputation) {

    this->dispatchComputation = dispatchComputation;

}

Handle<DispatchComp> getDispatchComputation () {

    return this->dispatchComputation;

}


private:

// the type of the job
// now there are two types of jobs:
// --DispatchData
// --Query
String jobType;

// the name of the job
String jobName;

//below attributes are for Query job

// the TCAP string of the job
String tcap;

//the list of Computation objects required for the job
Handle<Vector<Handle<Computation>>> allComputations;

//below attribute is for DispatchData job
Handle<DispatchComp> dispatchComputation;



};


}
#endif
