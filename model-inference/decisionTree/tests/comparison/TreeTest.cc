// This is a test program for loading a tree from a local file
// To compile the code and run it, just : $g++ -std=c++11 TreesTest.cc -o TreesTest
// And then run the program using the following rules:
// The first parameter specifies whether to classification ("C") or regression ("R").
// The second parameter specifies the number of inputs
// The third parameter specifies the batch size
// The fourth parameter specifies whether to use double("Y") or float("N")
// Starting with the fifth parameter, each parameter will represent one path of a tree. The following is a running example.
// $./TreeTest C 10000 100 Y ../higgs/higgs_0.txt ../higgs/higgs_1.txt
// $./TreeTest C 10000 100 N ../higgs/higgs_0.txt ../higgs/higgs_1.txt ../higgs/higgs_2.txt ../higgs/higgs_3.txt ../higgs/higgs_4.txt ../higgs/higgs_5.txt ../higgs/higgs_6.txt ../higgs/higgs_7.txt ../higgs/higgs_8.txt ../higgs/higgs_9.txt

#include <iostream>
#include <fstream>
#include <future>
#include <thread>
#include <unordered_map>
#include <sstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>
#include <cassert>
#include <memory>
#include <vector>
#include <algorithm>
#include <map>
#include <set>
#include <cstring>
#include <cstdio>
#include <exception>
#include <cassert>
#include <ctime>
#include <cstdlib>
#include <array>
#include <chrono>

using namespace std;

#define N 999

static float FLOAT_DEFAULT = 0.0;
static int INTEGER_DEFAULT = 0;
static bool BOOL_DEFAULT = false;

// this is the tree node class, which exactly the same as class Node in TreeNode.h under /src/decisionTree/headers/TreeNode.h
template<class T>
class Node{

	public:

		int nodeID = INTEGER_DEFAULT;
		int indexID = INTEGER_DEFAULT;
		bool isLeaf = BOOL_DEFAULT;
		int leftChild = INTEGER_DEFAULT;
		int rightChild = INTEGER_DEFAULT;
		// returnClass will be the vaule to compare while this is not a leaf node
		T returnClass = FLOAT_DEFAULT;

		Node() {}
		~Node() {}

		Node(int nodeIDIn, int indexIDIn, bool isLeafIn, int leftChildIn, int rightChildIn, float returnClassIn){
			nodeID = nodeIDIn;
			indexID = indexIDIn;
			isLeaf = isLeafIn;
			leftChild = leftChildIn;
			rightChild = rightChildIn;
			returnClass = returnClassIn;
		}

		int get_node() {
			return nodeID;
		}

		int get_index() {
			return indexID;
		}

		bool get_isLeaf() {
			return isLeaf;
		}

		int get_leftChild() {
			return leftChild;
		}

		int get_rightChild() {
			return rightChild;
		}

		T get_returnClass() {
			return returnClass;
		}

		void set_node(int nodeIDIn) {
			nodeID = nodeIDIn;
		}

		void set_index(int indexIDIn) {
			indexID = indexIDIn;
		}

		void set_isLeaf(bool isLeafIn) {
			isLeaf = isLeafIn;
		}

		void set_leftChild(int leftChildIn) {
			leftChild = leftChildIn;
		}

		void set_rightChild(int rightChildIn) {
			rightChild = rightChildIn;
		}

		void set_returnClass(float returnClassIn) {
			returnClass = returnClassIn;
		}

		bool operator == (const int &nodeID){
			return (this->nodeID == nodeID);
    }
};


//This is the forest class
template<class T>
class RandomForest{

private:

      std::vector<std::vector<Node<T>>> trees;

public:

      RandomForest() {}
      ~RandomForest() {}

      RandomForest(std::vector<std::string> inputFiles, std::string type) {
	  int numInputFiles = inputFiles.size();
	  for(int n = 0; n < numInputFiles; n++){
                std::string inputFileName = std::string(inputFiles[n]);
                std::ifstream infile;
                infile.open(inputFileName.data()); // link the ifstream with the file name
                assert(infile.is_open()); // stop the program if failed

                std:: string line;
                std::vector<std::string> relationships;
                std::vector<std::string> innerNodes;
                std::vector<std::string> leafNodes;
                string::size_type position;
                bool leftLine = true;
                bool rightLine = true;

                while(getline(infile,line))
                {
                        if(line == "digraph Tree {" || line == "node [shape=box] ;" || line == "}"){
                                continue;
                        }else{
                                position = line.find("->");
                                if (position != string::npos){
                                        if(line.find("0 -> 1") != string::npos && line.find("True") != string::npos){
                                                rightLine = false;
                                        } else{
                                                leftLine = true;
                                        }
                                        relationships.push_back(line);
                                }
                                else{
                                        if(line.find("X") != string::npos){
                                                innerNodes.push_back(line);
                                        } else{
                                                leafNodes.push_back(line);
                                        }
                                }
                        }
                }
                infile.close();
                int findStartPosition;
                int findMidPosition;
                int findEndPosition;
                std::vector<Node<T>> tree; 

                for (int i = 0; i < innerNodes.size(); i++) {
                    string thisLine = innerNodes[i];
                    int nodeID;
                    int indexID;
                    T returnClass;
                    if((findEndPosition = thisLine.find_first_of("[label=")) != string::npos){
                        nodeID = std::stoi(thisLine.substr(0, findEndPosition-1));
                    }
                    if((findStartPosition = thisLine.find("X")) != string::npos && (findEndPosition = thisLine.find_first_of("]")) != string::npos){
                        indexID = std::stoi(thisLine.substr(findStartPosition+2, findEndPosition-(findStartPosition+2)));
                    }
                    if((findStartPosition = thisLine.find("<=")) != string::npos && (findEndPosition = thisLine.find_first_of("ngini")) != string::npos){
                        returnClass = (T)(std::stof(thisLine.substr(findStartPosition+3, findEndPosition-1-(findStartPosition+3))));
                    }
                    tree.push_back(Node<T>(nodeID, indexID, false, -1, -1, returnClass));
                }

                for (int i = 0; i < leafNodes.size(); i++) {
                     string thisLine = leafNodes[i];
                     int nodeID;
                     T returnClass = -1.0;
                     if((findEndPosition = thisLine.find_first_of("[label=\"gini")) != string::npos){
                        nodeID = std::stoi(thisLine.substr(0, findEndPosition-1));
                     }
                     if(type == "C"){
                        if((findStartPosition = thisLine.find("nvalue = "))!= string::npos && (findMidPosition = thisLine.find_first_of(",")) != string::npos && (findEndPosition = thisLine.find_first_of("]\"] ;")) != string::npos){
                                T firstValue = (T)(std::stod(thisLine.substr(findStartPosition+10, findMidPosition-(findStartPosition+10))));
                                T secondValue = (T)(std::stod(thisLine.substr(findMidPosition+2, findEndPosition-(findMidPosition+2))));
                                if(firstValue >= secondValue){
                                        returnClass = 1.0;
                                }else{
                                        returnClass = 2.0;
                                }
                        }
                     } else if(type == "R"){
                        if((findStartPosition = thisLine.find("nvalue = "))!= string::npos && (findEndPosition = thisLine.find_last_of("\"] ;")) != string::npos){
                                returnClass = std::stod(thisLine.substr(findStartPosition+9, findEndPosition-(findStartPosition+9)));
                                //std::cout << returnClass << std::endl;
                        }
                     } else{
                        std::cout << "category error" << std::endl;
                        exit(1);
                     }
                     tree.push_back(Node<T>(nodeID, -1, true, -1, -1, returnClass));
                }

		for (int i = 0; i < relationships.size(); i++) {
                     int nodeID;
                     int thisChild;
                     string thisLine = relationships[i];
                     if((findMidPosition = thisLine.find_first_of("->")) != string::npos){
                        nodeID = std::stoi(thisLine.substr(0, findMidPosition-1));
                        //std::cout << nodeID << std::endl;
                     }
                     if(nodeID == 0){
                        if((findEndPosition = thisLine.find_first_of(" [")) != string::npos){
                                thisChild = std::stoi(thisLine.substr(findMidPosition+3, findEndPosition-1-(findMidPosition+3)));
                                //std::cout << thisChild << std::endl;
                        }
                     } else{
                        if((findEndPosition = thisLine.find_first_of(" ;")) != string::npos){
                                thisChild = std::stoi(thisLine.substr(findMidPosition+3, findEndPosition-1-(findMidPosition+3)));
                                //std::cout << thisChild << std::endl;
                        }
                     }
                     auto iter = find(tree.begin(), tree.end(), nodeID);
                     if(iter != tree.end()){
                        if(iter->get_leftChild() == -1){
                                iter->set_leftChild(thisChild);
                        } else{
                                iter->set_rightChild(thisChild);
                        }
                     } else{
                        std::cout << "construct error" << std::endl;
                        exit(1);
                     }
                }
                trees.push_back(tree);
	  }
	  std::cout << "numbers of trees in the forest: " << trees.size() << std::endl;
          std::cout << "numbers of nodes in each tree: " << std::endl;
          for(int i = 0; i < trees.size(); i++){
                std::cout << "numbers of nodes in tree " << i << " is " << trees[i].size() << std::endl;
          }
      }

      void predict(T * input, int inNumRow, int batchSize, int numFeatures, T * resultMatrix) {

     	 auto begin = std::chrono::high_resolution_clock::now();

         T inputValue;

	 Node<T> * treeNode = nullptr;

	 int numTrees = trees.size();

	 // do the inference
	 T * results = new T[numTrees];

	 for (int i = 0; i < inNumRow/batchSize; i++){

             for (int k = 0; k < batchSize; k++) {
    
        	 int index = i*batchSize+k;

		 //std::cout << "Now processing the " << index << "-th sample" << std::endl;
                 for(int j = 0; j < trees.size(); j++){

    	             // pass the root node of the tree
                     treeNode = & trees[j][0];
 
 		     while(treeNode->isLeaf == false){

        		 inputValue = input[index*numFeatures+treeNode->indexID];
#ifdef TREE_DEBUG
			 std::cout << "inputValue:" << inputValue << ", threshold:" << treeNode->returnClass << " ";
#endif
			 if(inputValue <= treeNode->returnClass){

   		    	      treeNode = treeNode + treeNode->leftChild;

			 }else{

                              treeNode = treeNode + treeNode->rightChild;
                         }
                     }
#ifdef TREE_DEBUG
		     std::cout << j << ":" << treeNode->returnClass << " ";
#endif
		     results[j] = treeNode->returnClass;
                  }

                  T voteResult = most_common(results, numTrees);

#ifdef TREE_DEBUG
		  std::cout << "\nVoted:" << voteResult << std::endl;
#endif
                  resultMatrix[index] = voteResult;
           }
         }
         auto end = std::chrono::high_resolution_clock::now();
         std::cout << typeid(T).name() << " type in C Inference Time: " << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count() << " secs." << std::endl;

      }


      int mostFrequent(T arr[], int n) {
         // Insert all elements in hash.
         unordered_map<T, int> hash;
         for (int i = 0; i < n; i++)
             hash[arr[i]]++;

         // find the max frequency
         int max_count = 0, res = -1;
         for (auto i : hash) {
            if (max_count < i.second) {
               res = i.first;
               max_count = i.second;
            }
         }
         return res;
      }


      float most_common(T input_array[], int n) {
         int t;
         T most_value;
         int max = -1;
         for (int i = 0; i < n; i++){
             t = 0;
             for (int j = 0; j < n; j++){
                 if(input_array[i]==input_array[j]){
                     ++t;
                 }
             }
             if (t > max){
                 max = t;
                 most_value = input_array[i];
             }
        }
        return most_value;
    }

};


int main(int argc, const char *argv[]){
	// "C" represents classification and "R" represents regression

	std::string type = std::string(argv[1]);

	// get the number of inputs
        int inNumRow = atoi(argv[2]);

	// get the batchSize 
        int batchSize = atoi(argv[3]);

        // feature size for each
 	const int inNumFeature = 28;	
        
	// whether to use double type
        bool isDouble = true;
	if (strcmp(argv[4], "N") == 0)
            isDouble = false;

	// input files to initialize trees
	std::vector<std::string> fileNames;
        for (int i = 5; i < argc; i++) {
	
            fileNames.push_back(std::string(argv[i]));
	
	}

	size_t numTrees = fileNames.size();

        srand(time(NULL));

	if (isDouble) {

             RandomForest<double> * myForest = new RandomForest<double>(fileNames, type);
	
             // randomly generate the inputs
             double * input = new double[inNumRow*inNumFeature];

    	     // seed
             for(int i = 0; i < inNumRow; i++){

     		     for(int j = 0; j < inNumFeature; j++){

	 		     input[i*inNumFeature+j] = (double)(rand() % (N + 1)) / (double)(N + 1);
                     }
	     }



             // set the final output array
             double * resultMatrix = new double[inNumRow];

	     myForest->predict(input, inNumRow, batchSize, inNumFeature, resultMatrix);

	     delete myForest;
             delete input, resultMatrix;

	} else {
	
             RandomForest<float> * myForest = new RandomForest<float>(fileNames, type);
        
             // randomly generate the inputs
             float *input = new float[inNumRow*inNumFeature];
    
             for(int i = 0; i < inNumRow; i++){
                
                     for(int j = 0; j < inNumFeature; j++){
                    
                             input[i*inNumFeature+j] = (float)(rand() % (N + 1)) / (float)(N + 1);
                     }
             }
     
    
             // set the final output array
             float * resultMatrix = new float[inNumRow];

             myForest->predict(input, inNumRow, batchSize, inNumFeature, resultMatrix);

	     delete myForest;
	     delete input, resultMatrix;
	}

    return 0;
}
