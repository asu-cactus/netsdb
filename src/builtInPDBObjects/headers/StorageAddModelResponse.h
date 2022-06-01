#ifndef STORAGEADDMODELRESPONSE_H
#define STORAGEADDMODELRESPONSE_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

// PRELOAD %StorageAddModelResponse%

namespace pdb {

// encapsulates a response to the request of adding a model in the storage (and catalog)
class StorageAddModelResponse : public Object {

public:
    StorageAddModelResponse() {}
    ~StorageAddModelResponse() {}

    //the parameter should refer to a path to the folder that contains text-based tree files converted from a scikit-learn ensemble model.
    StorageAddModelResponse(std::string pathToForest, std::string modelMaterializationType, std::string modelType, void * modelPointer) : 
	    pathToForest(pathToForest), modelMaterializationType(modelMaterializationType), modelType(modelType), modelPointer(modelPointer) {

	    }

    std::string getPathToForest() {
        return pathToForest;
    }

    std::string getModelMaterializationType() {
        return modelMaterializationType;
    }
   
    std::string getModelType() {
        return modelType;
    }
    
    void * getModelPointer() {
        return modelPointer;
    }

    ENABLE_DEEP_COPY

private:
    String pathToForest;

    //We plan to support three types:
    //(1) "compiled": the model is compiled to assembly code
    //(2) "fast": the model is stored to facilitate the RapidScorer algorithm
    //(3) "naive": the model is materialized for naive tree traversal
    //(4) "join": the model is materialized for join algorithm 
    String modelMaterializationType; 

    //We plan to support following models:
    //(1) "randomforest"
    //(2) "gdbt"
    //(3) "conv"
    //(4) "ffnn"
    String modelType;

    void * modelPointer=nullptr;

};
}

#endif  // STORAGEADDMODELRESPONSE_H
