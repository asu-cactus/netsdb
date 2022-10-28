
// Created by venkateshgunda on 5/10/22.
//
// Refactored by Jia to move most of the construction and prediction logic to this class and to apply more performance optimizations
//
#ifndef NETSDB_FOREST_H
#define NETSDB_FOREST_H

#define MAX_NUM_NODES_PER_TREE 512
#define MAX_NUM_TREES 1600

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

// PRELOAD %Forest%

using std::filesystem::directory_iterator;

namespace pdb
{

    typedef struct {
    
         int indexID;
         bool isLeaf;
         int leftChild;
         int rightChild;
         // returnClass will be the vaule to compare while this is not a leaf node
         float returnClass; 
    } Node;

    class Forest : public Object
    {
    public:
        ENABLE_DEEP_COPY

        Node forest[MAX_NUM_TREES][MAX_NUM_NODES_PER_TREE];
        int numTrees;
        ModelType modelType;

        Forest() {}

        Forest(ModelType type)
        {
            this->modelType = type;
        }

	Forest(std::string pathToFolder, ModelType modelType, bool isClassification) 
	{
	    this->constructForestFromFolder(pathToFolder, modelType, isClassification);
	
	}

	void constructForestFromFolder(std::string pathToFolder, ModelType modelType, bool isClassification) {
	
	    std::vector<std::string> treePaths;

            for (const auto & file : directory_iterator(pathToFolder)) {
	  
		  treePaths.push_back(file.path());  
	    
	    } 

	    constructForestFromPaths(treePaths, modelType, isClassification);
	
	}

        void processInnerNodes(std::vector<std::string> & innerNodes, ModelType modelType, int treeID)
	{


            int findStartPosition;
            int findMidPosition;
            int findEndPosition;

            for (int i = 0; i < innerNodes.size(); ++i)
            { 
	       // Construct Inner Nodes
               string currentLine = innerNodes[i];
               int nodeID;
               int indexID;
               float returnClass;

	       //to get nodeID
	       if (modelType == ModelType::RandomForest) {
                   if ((findEndPosition = currentLine.find_first_of("[label")) != string::npos)
                   {
                       nodeID = std::stoi(currentLine.substr(0, findEndPosition - 1));
                   }
		   if ((findStartPosition = currentLine.find("X[")) != string::npos && (findEndPosition = currentLine.find("] <=")) != string::npos)
                    { 
                        indexID = std::stoi(currentLine.substr(findStartPosition + 2, findEndPosition));
                    }

		   if ((findStartPosition = currentLine.find("<=")) != string::npos && (findEndPosition = currentLine.find_first_of("\\ngini")) != string::npos)
                    { 
                        returnClass = std::stod(currentLine.substr(findStartPosition + 3, findEndPosition));

                    }
	       } else {
	           if ((findEndPosition = currentLine.find_first_of("[ label")) != string::npos)
                   {
                       nodeID = std::stoi(currentLine.substr(4, findEndPosition - 1));
                   }
		   if ((findStartPosition = currentLine.find("f")) != string::npos && (findEndPosition = currentLine.find("<")) != string::npos)
                    { 
                        indexID = std::stoi(currentLine.substr(findStartPosition + 1, findEndPosition));
                    }
		   if ((findStartPosition = currentLine.find("<")) != string::npos && (findEndPosition = currentLine.find_first_of("]")) != string::npos)
                    { 
                        returnClass = std::stod(currentLine.substr(findStartPosition + 1, findEndPosition));
                    }
	       }
	       forest[treeID][nodeID].indexID = indexID;
               forest[treeID][nodeID].isLeaf = false;         
               forest[treeID][nodeID].leftChild = -1;
               forest[treeID][nodeID].rightChild = -1;
               forest[treeID][nodeID].returnClass = returnClass;
           }

	}


	void processLeafNodes(std::vector<std::string> & leafNodes, ModelType modelType, int treeID)
        {

            int findStartPosition;
            int findMidPosition;
            int findEndPosition;

            for (int i = 0; i < leafNodes.size(); ++i)
            { 
		// Construct Leaf Nodes
                string currentLine = leafNodes[i];
                int nodeID;
                float returnClass = -1.0f;
		if (modelType == ModelType::RandomForest) {
                    if ((findEndPosition = currentLine.find_first_of("[label=\"gini")) != string::npos)
                    {
                        nodeID = std::stoi(currentLine.substr(0, findEndPosition - 1));
                    }
                    // Output Class of always a Double/Float. ProbabilityValue for Classification, ResultValue for Regression
                    if ((findStartPosition = currentLine.find("y[")) != string::npos && (findEndPosition = currentLine.find("]\"]")) != string::npos)
                    {
                        returnClass = std::stod(currentLine.substr(findStartPosition + 2, findEndPosition));
                    }
                } else {

	            if ((findEndPosition = currentLine.find_first_of("[")) != string::npos)
                    {
                        nodeID = std::stoi(currentLine.substr(4, findEndPosition-1));
                    }
                    // Output Class of XGBoost always a Double/Float. ProbabilityValue for Classification, ResultValue for Regression
                    if ((findStartPosition = currentLine.find("leaf=")) != string::npos && (findEndPosition = currentLine.find("]")) != string::npos)
                    {
                        returnClass = std::stod(currentLine.substr(findStartPosition+5, findEndPosition-3));
                    }	
		
        	}
                forest[treeID][nodeID].indexID = -1;
                forest[treeID][nodeID].isLeaf = true;
                forest[treeID][nodeID].leftChild = -1;
                forest[treeID][nodeID].rightChild = -1;
                forest[treeID][nodeID].returnClass = returnClass;
	    }
        }

	void processRelationships(std::vector<std::string> & relationships, ModelType modelType, int treeID)
        {

                int findStartPosition;
                int findMidPosition;
                int findEndPosition;

                for (int i = 0; i < relationships.size(); ++i)
                { // Construct Directed Edges between Nodes
                    int parentNodeID;
                    int childNodeID;
                    std::string currentLine = relationships[i];

                    if (modelType == ModelType::RandomForest) {
                        if ((findMidPosition = currentLine.find_first_of("->")) != std::string::npos)
                        {
                            parentNodeID = std::stoi(currentLine.substr(0, findMidPosition-1));
                        }
			if (parentNodeID == 0) {
			
			    if ((findEndPosition = currentLine.find_first_of("[label")) != std::string::npos) {

                                childNodeID = std::stoi(currentLine.substr(findMidPosition + 3, findEndPosition - 1));

                            }
			} else {
			
			    if ((findEndPosition = currentLine.find_first_of(";")) != std::string::npos) {

                                childNodeID = std::stoi(currentLine.substr(findMidPosition + 3, findEndPosition-1));

                            }
			
			}

		    } else {

		        if ((findMidPosition = currentLine.find_first_of("->")) != std::string::npos) {

                            parentNodeID = std::stoi(currentLine.substr(4, findMidPosition - 1));
                        }
			if ((findEndPosition = currentLine.find_first_of("[")) != std::string::npos) {

                                childNodeID = std::stoi(currentLine.substr(findMidPosition + 3, findEndPosition-1));
                        }
		    
		    }

                    if (forest[treeID][parentNodeID].leftChild == -1)
                    {
                        forest[treeID][parentNodeID].leftChild = childNodeID;
                    }
                    else
                    {
                        forest[treeID][parentNodeID].rightChild = childNodeID;
                    }

                }


        }

        void constructForestFromPaths(std::vector<std::string> & treePathIn, ModelType modelType, bool isClassification) {

	    this->modelType = modelType;
            this->numTrees = treePathIn.size();

            for (int n = 0; n < numTrees; ++n)
            {
                std::string inputFileName = std::string(treePathIn[n]);
		std::cout << inputFileName << std::endl;
                std::ifstream inputFile;
                inputFile.open(inputFileName.data());
                assert(inputFile.is_open());

                std::string line;
                std::vector<std::string> relationships;
                std::vector<std::string> innerNodes;
                std::vector<std::string> leafNodes;
                string::size_type position;

                while (getline(inputFile, line))
                {
                    if ((line.size()==0) || (line.find("graph")!=std::string::npos) || (line.find("digraph Tree {")!=std::string::npos) || (line.find("node [shape=box] ;")!=std::string::npos) || (line.find("}")!=std::string::npos))
                    {
                        continue;
                    }
                    else
                    {
                        position = line.find("->");
                        if (position != string::npos)
                        {
                            relationships.push_back(line);
                        }
                        else
                        { // Find Leaf/Inner Node
                            if ((line.find("leaf") != string::npos) || (line.find("[label=\"gini") != string::npos))
                            {
                                leafNodes.push_back(line);
                            }
                            else
                            {
                                innerNodes.push_back(line);
                            }
                        }
                    }
                }

    		inputFile.close();

                processInnerNodes(innerNodes, modelType, n);
                processLeafNodes(leafNodes, modelType, n);
		processRelationships(relationships, modelType, n);

            }

            // STATS ABOUT THE FOREST
            std::cout << "Number of trees in the forest: " << numTrees << std::endl;
	}


        template <class InputIt, class T = typename std::iterator_traits<InputIt>::value_type>
        T most_common(InputIt begin, InputIt end)
        {
            std::map<T, int> counts;
            for (InputIt it = begin; it != end; ++it)
            {
                if (counts.find(*it) != counts.end())
                {
                    ++counts[*it];
                }
                else
                {
                    counts[*it] = 1;
                }
            }
            return std::max_element(counts.begin(), counts.end(), [](const std::pair<T, int> &pair1, const std::pair<T, int> &pair2)
                                    { return pair1.second < pair2.second; })
                ->first;
        }

        // Decision of an XGBoost Tree (for class-1, not class-0): sigmoid(log(previous_tree_pred(initial_value=0)/(1-previous_tree_pred(initial_value=0))) + learning_rate*(current_tree_prob))
        template <class InputIt, class T = typename std::iterator_traits<InputIt>::value_type>
        T aggregate_decisions(InputIt begin, InputIt end)
        {
            // Default LR Value Source: https://xgboost.readthedocs.io/en/stable/parameter.html?highlight=0.3#parameters-for-tree-booster
            double learning_rate = 0.3;       // TODO: Hard-coding XGBoost Library default value. Change this to a parameter if modified.
            double aggregated_decision = 0.0; // Initial Prediction is 0.5, which makes log(odds) value 0. If different, the default value changes.
            double threshold = 0.5;
            for (InputIt it = begin; it != end; ++it)
            {
                aggregated_decision += (*it); // Should not be multiplied by Learning Rate
            }
            // Reference: https://stats.stackexchange.com/questions/395697/what-is-an-intuitive-interpretation-of-the-leaf-values-in-xgboost-base-learners
            double sigmoid_of_decision = 1 / (1 + exp(-1.0 * aggregated_decision)); // Sigmoid of the aggregated decision is the final output
            return sigmoid_of_decision > threshold ? 2.0 : 1.0;                     // Delaying Class Assignment. Class Labels as per RF Code is 1.0 and 2.0
        }

        // TODO: Call this function instead of Individual Compute Functions. Instantiate the Class with the Model Type
        template <class InputIt, class T = typename std::iterator_traits<InputIt>::value_type>
        T compute_result(InputIt begin, InputIt end)
        {
            switch (modelType)
            {
            case ModelType::RandomForest:
                return most_common(begin, end);
            case ModelType::XGBoost:
                return aggregate_decisions(begin, end);
            case ModelType::LightGBM:
                return aggregate_decisions(begin, end);
            default: // RF is default
                return most_common(begin, end);
            }
        }

        template <class T>
        pdb::Handle<pdb::Vector<T>> predict(Handle<TensorBlock2D<T>> &in)
        {                                 // TODO: Change all Double References to Float

            // get the input features matrix information
            uint32_t inNumRow = in->getRowNums();
	    uint32_t inNumCol = in->getColNums();

            T *inData = in->getValue().rawData->c_ptr();

            // set the output matrix
            pdb::Handle<pdb::Vector<T>> resultMatrix = pdb::makeObject<pdb::Vector<T>>(inNumRow, inNumRow);
	    std::vector<T> thisResultMatrix (numTrees);

	    T * outData = resultMatrix->c_ptr();

            T inputValue;
	    Node treeNode;
	    int featureStartIndex;
	    int curIndex;

            for (int i = 0; i < inNumRow; i++)
            {
		featureStartIndex = i*inNumCol;
                for (int j = 0; j < numTrees; j++)
                {
                    //  inference
                    //  pass the root node of the tree
		    Node * tree = forest[j];
                    treeNode = tree[0];
                    while (treeNode.isLeaf == false)
                    {

                        if (inData[featureStartIndex + treeNode.indexID] <= treeNode.returnClass)
                        {
			    treeNode = tree[treeNode.leftChild];
                        }
                        else
                        {
			    treeNode = tree[treeNode.rightChild];
                        }
                    }
                    thisResultMatrix[j] = treeNode.returnClass;
                }
                T voteResult = compute_result(thisResultMatrix.begin(), thisResultMatrix.end());
		outData[i]=voteResult;
	    }
            return resultMatrix;
        }
    };
}

#endif // NETSDB_FOREST_H
