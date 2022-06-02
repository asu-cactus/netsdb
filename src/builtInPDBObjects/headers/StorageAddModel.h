#ifndef STORAGEADDMODEL_H
#define STORAGEADDMODEL_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

// PRELOAD %StorageAddModel%

namespace pdb {

// encapsulates a request to add a model in the local storage (and catalog)
class StorageAddModel : public Object {

public:
    StorageAddModel() {}
    ~StorageAddModel() {}

    //the parameter should refer to a path to the folder that contains text-based tree files converted from a scikit-learn ensemble model.
    StorageAddModel(std::string pathToModel, std::string modelMaterializationType, std::string modelType) : 
	    pathToModel(pathToModel), modelMaterializationType(modelMaterializationType), modelType(modelType) {}

    std::string getPathToModel() {
        return pathToModel;
    }

    std::string getModelMaterializationType() {
        return modelMaterializationType;
    }

    std::string getModelType() {
        return modelType;
    }

    ENABLE_DEEP_COPY

private:
    String pathToModel;

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

};
}

#endif  // STORAGEADDMODEL_H
