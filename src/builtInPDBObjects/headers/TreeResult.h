
#ifndef NETSDB_TREE_RESULT_H
#define NETSDB_TREE_RESULT_H

#define BLOCK_SIZE 275000


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
             for (int i = 0; i < BLOCK_SIZE; i++) {
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

        float data[BLOCK_SIZE];
	
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

            for (int i = 0; i < BLOCK_SIZE; i++) {
                  (myData)[i] += (otherData)[i];
	    }
		
            return *this;
        }
        int getNumPositives () {
	
	    int positive_count = 0;
            for (int i = 0; i < BLOCK_SIZE; i++) {
   		    positive_count += data[i]-1.0; 
	    }
            return positive_count;	
	}
	void postprocessing(Handle<TreeResult> res) {
	
	   float * resData = res->data;   
	   float threshold = 0.5;
	   float sigmoid_of_decision;
	   // Default LR Value Source: https://xgboost.readthedocs.io/en/stable/parameter.html?highlight=0.3#parameters-for-tree-booster
            for (int i = 0; i < BLOCK_SIZE; i++) {
                sigmoid_of_decision = 1 / (1 + exp(-1.0 * data[i]));
		resData[i] = sigmoid_of_decision > threshold ? 2.0 : 1.0;
	    }
            // Reference: https://stats.stackexchange.com/questions/395697/what-is-an-intuitive-interpretation-of-the-leaf-values-in-xgboost-base-learners
	}

    };
}

#endif // NETSDB_TREE_RESULT_H
