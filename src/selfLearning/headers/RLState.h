#ifndef RL_STATE_H
#define RL_STATE_H

#include <vector>

#define NUM_CANDIDATE_LAMBDA 2 

namespace pdb {

//defines the state space


class RLState {

public:

     double dataDistance[NUM_CANDIDATE_LAMBDA] = {0.0};
     double joinDistance[NUM_CANDIDATE_LAMBDA] = {0.0};
     double aggDistance[NUM_CANDIDATE_LAMBDA] = {0.0};
     double frequency[NUM_CANDIDATE_LAMBDA] = {0.0};
     double selectivity[NUM_CANDIDATE_LAMBDA] = {0.0};
     double avgNumHashKeys[NUM_CANDIDATE_LAMBDA] = {0.0};
     double inputSize = 0.0;
     double numNodes = 0.0; 
     double numTotalPartitions = 0.0;
     double numCandidates = 0.0;

     void toVector(std::vector<double>& myVec){
         myVec.clear();
         for (int i = 0; i < NUM_CANDIDATE_LAMBDA; i++) {
             myVec.push_back(dataDistance[i]);
         }
         for (int i = 0; i < NUM_CANDIDATE_LAMBDA; i++) {
             myVec.push_back(joinDistance[i]);
         }
         for (int i = 0; i < NUM_CANDIDATE_LAMBDA; i++) {
             myVec.push_back(aggDistance[i]);
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
         myVec.push_back(numTotalPartitions);
         myVec.push_back(numCandidates);
     }

     void print(){
         for (int i = 0; i < NUM_CANDIDATE_LAMBDA; i++) {
             std::cout << "dataDistance[" << i << "]=" << dataDistance[i] << std::endl;
         }
         for (int i = 0; i < NUM_CANDIDATE_LAMBDA; i++) {
             std::cout << "joinDistance[" << i << "]=" << joinDistance[i] << std::endl;
         }
         for (int i = 0; i < NUM_CANDIDATE_LAMBDA; i++) {
             std::cout << "aggDistance[" << i << "]=" << aggDistance[i] << std::endl;
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
         std::cout << "numTotalPartitions = " << numTotalPartitions << std::endl;
         std::cout << "numCandidates = " << numCandidates << std::endl;
     }

};

}

#endif
