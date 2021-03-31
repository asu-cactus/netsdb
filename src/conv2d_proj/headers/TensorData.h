#ifndef TENSORDATA_H
#define TENSORDATA_H

#include "Handle.h"
#include "Object.h"
#include "PDBVector.h"

class TensorData : public pdb::Object {
public:

  ENABLE_DEEP_COPY

  int numRanks = 3;

  //always follows NCHW format  
  pdb::Handle<pdb::Vector<unsigned int>> dimensions;

  //always follows NCHW format
  pdb::Handle<pdb::Vector<float>> rawData;

  ~TensorData() {}

  //initialize raw data as zeros;
  TensorData(int numRanksIn, pdb::Handle<pdb::Vector<unsigned int>> dimensionsIn)
      : numRanks(numRanksIn), dimensions(dimensionsIn) {

    int size = 1;

    for (unsigned int i = 0; i < numRanksIn; i ++) {

        size = size * (*dimensions)[i];

    }

    rawData = pdb::makeObject<pdb::Vector<float>>(size, size);

  }

  //initialize raw data using external data
  TensorData(int numRanksIn, pdb::Handle<pdb::Vector<unsigned int>> dimensionsIn,
               pdb::Handle<pdb::Vector<float>> rawDataIn)
      : numRanks(numRanksIn), dimensions(dimensionsIn), rawData(rawDataIn) {
  }

  TensorData() {}

};

#endif
