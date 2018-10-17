#ifndef RL_STATE_H
#define RL_STATE_H

#include <vector>
#include <cfloat>
#define NUM_CANDIDATE_LAMBDA 2 

namespace pdb {

//defines the state space


class RLState {

public:

     int candidateLambdaIndex[NUM_CANDIDATE_LAMBDA] = {-1};
     double dataDistance[NUM_CANDIDATE_LAMBDA] = {0.0};
     double joinDistance[NUM_CANDIDATE_LAMBDA] = {0.0};
     double aggDistance[NUM_CANDIDATE_LAMBDA] = {0.0};
     double frequency[NUM_CANDIDATE_LAMBDA] = {0.0};
     double selectivity[NUM_CANDIDATE_LAMBDA] = {0.0};
     double avgNumHashKeys[NUM_CANDIDATE_LAMBDA] = {0.0};
     double inputSize = 0.0;
     double dataScale = 0.0;
     int environmentId = -1;
     double numNodes = 0.0; 
     double numTotalPartitions = 0.0;
     double numCores = 0.0;
     double memSize = 0.0;
     double networkBandwidth = 0.0;
     double diskSpeed = 0.0;
     double numCandidates = 0.0;
     int indexWithLeastFrequency = -1;
     double leastFrequency = DBL_MAX;

     void clear() {
        for (int i = 0; i < NUM_CANDIDATE_LAMBDA; i++) {
            candidateLambdaIndex[i] = -1;
            dataDistance[i] = 0.0;
            frequency[i] = 0.0;
            selectivity[i] = 0.0;
            avgNumHashKeys[i] = 0.0;
        }
        inputSize = 0.0;
        dataScale = 0.0;
        numNodes = 0.0;
        numTotalPartitions = 0.0;
        numCores = 0.0;
        memSize = 0.0;
        networkBandwidth = 0.0;
        diskSpeed = 0.0;
        numCandidates = 0.0;
        indexWithLeastFrequency = -1;
        leastFrequency = DBL_MAX;
        environmentId = -1;
    }

    int getLambdaId(int index) {
        std::cout << "index is " << index << std::endl;
        if ((index < 0 ) || (index >= numCandidates)) {
            std::cout << "action lambda id is -1" << std::endl;
            return -1;
        } else {
            std::cout << "action lambda id is " << candidateLambdaIndex[index] << std::endl;
            return candidateLambdaIndex[index];
        }
    }

    void toVector(std::vector<double>& myVec){
         myVec.clear();
         for (int i = 0; i < NUM_CANDIDATE_LAMBDA; i++) {
             myVec.push_back(dataDistance[i]);
         }
         for (int i = 0; i < NUM_CANDIDATE_LAMBDA; i++) {
             myVec.push_back(frequency[i]);
         }
         for (int i = 0; i < NUM_CANDIDATE_LAMBDA; i++) {
             myVec.push_back(selectivity[i]);
         }
         for (int i = 0; i < NUM_CANDIDATE_LAMBDA; i++) {
             myVec.push_back(avgNumHashKeys[i]);
         }
         myVec.push_back(inputSize);
         myVec.push_back(numNodes);
         //myVec.push_back(numTotalPartitions);
         myVec.push_back(numCandidates);
         myVec.push_back(numCores);
         myVec.push_back(memSize);
         myVec.push_back(networkBandwidth);
         myVec.push_back(diskSpeed);
     }

     void print(){

         for (int i = 0; i < NUM_CANDIDATE_LAMBDA; i++) {
             std::cout << "candidateLambdaIndex[" << i << "]=" << candidateLambdaIndex[i] << std::endl;
         }

         for (int i = 0; i < NUM_CANDIDATE_LAMBDA; i++) {
             std::cout << "dataDistance[" << i << "]=" << dataDistance[i] << std::endl;
         }
         for (int i = 0; i < NUM_CANDIDATE_LAMBDA; i++) {
             std::cout << "frequency[" << i << "]=" << frequency[i] << std::endl;
         }
         for (int i = 0; i < NUM_CANDIDATE_LAMBDA; i++) {
             std::cout << "selectivity[" << i << "]=" << selectivity[i] << std::endl;
         }
         for (int i = 0; i < NUM_CANDIDATE_LAMBDA; i++) {
             std::cout << "avgNumHashKeys[" << i << "]=" << avgNumHashKeys[i] << std::endl;
         }
         std::cout << "inputSize = " << inputSize << std::endl;
         std::cout << "numNodes = " << numNodes << std::endl;
         //std::cout << "numTotalPartitions = " << numTotalPartitions << std::endl;
         std::cout << "numCores = " << numCores << std::endl;
         std::cout << "memSize = " << memSize << std::endl;
         std::cout << "networkBandwidth = " << networkBandwidth << std::endl;
         std::cout << "diskSpeed = " << diskSpeed << std::endl;
         std::cout << "numCandidates = " << numCandidates << std::endl;
     }

};

}

#endif
