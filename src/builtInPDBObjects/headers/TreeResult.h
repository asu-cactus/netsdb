
#ifndef NETSDB_TREE_RESULT_H
#define NETSDB_TREE_RESULT_H

//#define MAX_BLOCK_SIZE 2876726 
//#define MAX_BLOCK_SIZE 275000
#define MAX_BLOCK_SIZE 500000
//#define MAX_BLOCK_SIZE 1000000

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
        void setUpAndCopyFrom(void* target, void* source) const override {
             new (target) TreeResult ();
             TreeResult& fromMe = *((TreeResult *) source);
             TreeResult& toMe = *((TreeResult *) target);
	     toMe.blockSize = fromMe.blockSize;
             for (int i = 0; i < fromMe.blockSize; i++) {
                 toMe.data[i] = fromMe.data[i];
             }
             toMe.treeId = fromMe.treeId;
             toMe.modelType = fromMe.modelType;
             toMe.batchId = fromMe.batchId;
        }

         void deleteObject(void* deleteMe) override {
             deleter(deleteMe, this);
         }

         size_t getSize(void* forMe) override {
             return sizeof(TreeResult);
         }

        float data[MAX_BLOCK_SIZE];
	
	int treeId;

	int batchId;

	int blockSize;


	ModelType modelType;

        TreeResult() {}


	TreeResult(int treeId, int batchId, int blockSize, ModelType modelType) 
	{
            if (blockSize > MAX_BLOCK_SIZE) {
	    
	        std::cout << "FATAL ERROR: TreeResult: blockSize exceeds MAX_BLOCK_SIZE. Try increase MAX_BLOCK_SIZE" << std::endl;
		exit(1);

	    }
	    
	    this->treeId = treeId;
	    this->batchId = batchId;
	    this->blockSize = blockSize;
	    this->modelType = modelType;
	}

	int & getKey() {
	    return batchId;
	
	}

        void print() {
	
	    std::cout << "TreeResult: " << treeId << ":" << batchId << std::endl;

	}


	TreeResult & getValue() {
	    return *this; 
	
	}

	TreeResult &operator+(TreeResult &other) {
    
	    float *myData, *otherData;

            myData = this->data;
            otherData = other.data;

            for (int i = 0; i < blockSize; i++) {
                  myData[i] += otherData[i];
	    }
            return *this;
        }
        int getNumPositives () {
	    int positive_count = 0;
            for (int i = 0; i < blockSize; i++) {
   		    positive_count += data[i]; 
	    }
            return positive_count;	
	}
	void postprocessing(Handle<TreeResult> res, int numTrees = 0) {
	   float * resData = res->data;   

           if ((modelType == ModelType::XGBoost) || (modelType == ModelType::LightGBM)) {
	       float threshold = 0.5;
	       float sigmoid_of_decision;
	       // Default LR Value Source: https://xgboost.readthedocs.io/en/stable/parameter.html?highlight=0.3#parameters-for-tree-booster
               for (int i = 0; i < blockSize; i++) {
                   sigmoid_of_decision = 1 / (1 + exp(-1.0 * data[i]));
		   resData[i] = sigmoid_of_decision > threshold ? 1.0 : 0.0;
                   //std::cout << resData[i] << "," << data[i] << std::endl;
	       }
               // Reference: https://stats.stackexchange.com/questions/395697/what-is-an-intuitive-interpretation-of-the-leaf-values-in-xgboost-base-learners
	   } else {
	       for (int i = 0; i < blockSize; i++) {
	           resData[i] = data[i] > numTrees/2 ? 1.0 : 0.0;
	       }
	   }
	}

    };
}

#endif // NETSDB_TREE_RESULT_H
