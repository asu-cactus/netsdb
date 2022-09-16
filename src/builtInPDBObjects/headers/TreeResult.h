
#ifndef NETSDB_TREE_RESULT_H
#define NETSDB_TREE_RESULT_H

#define BATCH_SIZE 27500


#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <string>
#include <filesystem>
#include <future>
#include <thread>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cassert>
#include <memory>
#include <algorithm>
#include <map>
#include <set>
#include "TensorBlock2D.h"
#include "PDBClient.h"
#include "StorageClient.h"
#include "DataTypes.h"

// PRELOAD %TreeResult%

//this class is used to store the prediction results of a single decision tree on a batch of data
namespace pdb
{

    class TreeResult : public Object
    {
    public:
        ENABLE_DEEP_COPY

        float data[BATCH_SIZE];
	
	int treeId;

	int batchId;

	ModelType modelType;

        TreeResult() {}


	TreeResult(int treeId, int batchId, ModelType modelType) 
	{
	    this->treeId = treeId;
	    this->batchId = batchId;
	    this->modelType = modelType;
	
	}

	float * getData() {
	
            return data;
	
	}

	int & getKey() {
	
	    return batchId;
	
	} 

	TreeResult & getValue() {
	
	    return *this; 
	
	}

	TreeResult &operator+(TreeResult &other) {
    
	    float *myData, *otherData;

            myData = this->data;
            otherData = other.data;

	    if ((modelType == ModelType::XGBoost) || (modelType == ModelType::LightGBM))  { 
                for (int i = 0; i < BATCH_SIZE; i++) {
                      (myData)[i] += (otherData)[i];
	        }
	    }
	    else {
	    
		std::cout << "We do not support aggregation for RandomForest at this point" << std::endl;
	    
	    }

           return *this;
        }

    };
}

#endif // NETSDB_TREE_RESULT_H
