#ifndef DISTRIBUTEDSTORAGEADDMODEL_H
#define DISTRIBUTEDSTORAGEADDMODEL_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

// PRELOAD %DistributedStorageAddModel%

namespace pdb {

// encapsulates a request to add a model in the storage (and catalog)
class DistributedStorageAddModel : public Object {

public:
    DistributedStorageAddModel() {}
    ~DistributedStorageAddModel() {}

    //the parameter should refer to a path to the folder that contains text-based tree files converted from a scikit-learn ensemble model.
    DistributedStorageAddModel(std::string pathToModel, std::string modelMaterializationType, std::string modelType) : 
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
    String modelMaterializationType; 

    //We plan to support following models:
    //(1) "randomforest"
    //(2) "gdbt"
    //(3) "conv"
    //(4) "ffnn"
    String modelType;

};
}

#endif  // DISTRIBUTEDSTORAGEADDMODEL_H
