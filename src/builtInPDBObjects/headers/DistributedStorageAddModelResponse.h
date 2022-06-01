#ifndef DISTRIBUTEDSTORAGEADDMODELRESPONSE_H
#define DISTRIBUTEDSTORAGEADDMODELRESPONSE_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

// PRELOAD %DistributedStorageAddModelResponse%

namespace pdb {

// encapsulates a response to the request of adding a model in the storage (and catalog)
class DistributedStorageAddModelResponse : public Object {

public:
    DistributedStorageAddModelResponse() {}
    ~DistributedStorageAddModelResponse() {}

    //the parameter should refer to a path to the folder that contains text-based tree files converted from a scikit-learn ensemble model.
    DistributedStorageAddModelResponse(std::string pathToModel, std::string modelMaterializationType, std::string modelType, std::vector<void *>& modelPointers) : 
	    pathToModel(pathToModel), modelMaterializationType(modelMaterializationType), modelType(modelType) {

	          for (size_t i = 0; i < modelPointers.size(); i++) {
		  
		      this->modelPointers.push_back(modelPointers[i]);

		  }	    
	    
	    }

    std::string getPathToModel() {
        return pathToModel;
    }

    std::string getModelMaterializationType() {
        return modelMaterializationType;
    }

    std::string getModelType() {
        return modelType;
    }
    
    
    Vector<void *> modelPointers;

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

#endif  // DISTRIBUTEDSTORAGEADDMODELRESPONSE_H
