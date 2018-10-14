#ifndef SELF_LEARNING_CONTEXT_H
#define SELF_LEARNING_CONTEXT_H


// This class encapsulates all information that is required for automatic data placement optimization and automatic data eviction.
// A SelfLearningContext object will be passed between storage and computation layer for cross-layer optimization.

#include "Object.h"
#include "PDBString.h"

namespace pdb {

class SelfLearningContext {

public:

//constructor
SelfLearningContext (std::string clientName, unsigned int jobStageID, std::string jobStageType, 
       std::string setRole, unsigned int setTypeID, std::string databaseName, std::string setName);

//setters/getters

void setClientName(std::string clientName) {

    this->clientName = clientName;

}

std::string getClientName() {

    return this->clientName;

}

void setJobID(std::string jobID) {

    this->jobID = jobID;

}

std::string getJobID() {

    return this->jobID;

}

void setJobStageID(unsigned int jobStageID) {

    this->jobStageID = jobStageID;

}

unsigned int getJobStageID() {

    return this->jobStageID;

}

void setJobStageType(std::string jobStageType) {

    this->jobStageType = jobStageType;

}

std::string getJobStageType() {

    return this->jobStageType;

}

void setSetRole(std::string setRole) {

    this->setRole = setRole;

}

std::string getSetRole() {

    return this->setRole;

}

void setSetTypeID(unsigned int setTypeID) {

    this->setTypeID = setTypeID;

}
unsigned int getSetTypeID() {

    return this->setTypeID;

}

void setSetTypeName(std::string setTypeName) {

    this->setTypeName = setTypeName;

}

std::string getSetTypeName() {

    return this->setTypeName;

}

void setDatabaseName(std::string databaseName) {

    this->databaseName = databaseName;

}

std::string getDatabaseName() {

    return this->databaseName;

}

void setSetName(std::string setName) {

    this->setName = setName;

}

std::string getSetName() {

    return this->setName;

}

private:

//information from computation layer

//the user-provided name of client
String clientName;

//the JobID, each execution of the same query will have a unique JobID
String jobID;

//the id of JobStage
unsigned int jobStageID;

//the type of JobStage
String jobStageType;

//the role of the dataset to be requested;
String setRole;

//the PDB type id of the dataset to be requested;
unsigned int setTypeID;

//the type name of the dataset
String setTypeName;

//information from storage layer

//the database name
String databaseName;

//the set name
String setName;


};

}

#endif
