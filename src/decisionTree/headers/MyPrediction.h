#ifndef MY_PREDICTION_H
#define MY_PREDICTION_H

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <future>
#include <thread>
#include <sstream>
#include <string>
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
#include <cstring>
#include <exception>

#include "Object.h"
#include "PDBVector.h"
#include "PDBString.h"
#include "Handle.h"
#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "SelectionComp.h"
#include "BaselineNode.h"

using namespace pdb;

class MyPrediction: public SelectionComp<double, std::vector<double>> {
  private:
  	decisiontree::GenericNode topNode;

  public:

    ENABLE_DEEP_COPY

    MyPrediction() {}
    ~MyPrediction() {}

    MyPrediction(decisiontree::GenericNode topNodeIn) {
    	topNode = topNodeIn;
    }

    Lambda<bool> getSelection(Handle<std::vector<double>> checkMe) override {
        return makeLambda(checkMe,
                          [](Handle<std::vector<double>> &checkMe) { return true; });
    }

    Lambda<Handle<double>>
    getProjection(Handle<std::vector<double>> in) override {
        return makeLambda(in, [this](Handle<std::vector<double>> &in) {
        	std::vector<double> predictIn;
        	int vecSize = in->size();
        	for(int i=0; i < vecSize; i++){
        		double thisFeature = (*in)[i];
        		predictIn.push_back(thisFeature);
        	}
        	pdb::Handle<double> result = pdb::makeObject<double>(topNode.predict(predictIn));
        	return result;
        });
    }
};

#endif