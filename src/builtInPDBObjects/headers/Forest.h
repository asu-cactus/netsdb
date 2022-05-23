//
// Created by venkateshgunda on 5/10/22.
//
// Refactored by Jia to move most of the construction and prediction logic to this class and to apply more performance optimizations
//
#ifndef NETSDB_FOREST_H
#define NETSDB_FOREST_H

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
#include "Node.h"
#include "PDBClient.h"
#include "StorageClient.h"

// PRELOAD %Forest%

using std::filesystem::directory_iterator;

namespace pdb
{

    class Forest : public Object
    {
    public:
        ENABLE_DEEP_COPY

        pdb::Vector<pdb::Vector<pdb::Handle<pdb::Node>>> forest;
        int numTrees;
        ModelType modelType;

        Forest() {}

        Forest(ModelType type)
        {
            this->modelType = type;
        }

        Forest(pdb::Vector<pdb::Vector<pdb::Handle<pdb::Node>>> forestIn, ModelType type)
        {
            this->forest = forestIn;
            this->numTrees = forestIn.size();
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

	void constructForestFromPaths(std::vector<std::string> & treePathIn, ModelType modelType, bool isClassification) {

	    this->modelType = modelType;
            this->numTrees = treePathIn.size();

            for (int n = 0; n < numTrees; ++n)
            {
		std::cout << "Processing the tree at " << treePathIn[n] << std::endl;    
                std::string inputFileName = std::string(treePathIn[n]);
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
                    if ((line.find("digraph Tree {")!=std::string::npos) || (line.find("node [shape=box] ;")!=std::string::npos) || (line.find("}")!=std::string::npos))
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
                            if (line.find("leaf") != string::npos)
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

                int findStartPosition;
                int findMidPosition;
                int findEndPosition;
                pdb::Vector<pdb::Handle<pdb::Node>> tree;

		std::cout << "Processing inner nodes" << std::endl;
                for (int i = 0; i < innerNodes.size(); ++i)
                { // Construct Inner Nodes
                    string currentLine = innerNodes[i];
                    std::cout << currentLine << std::endl;
		    int nodeID;
                    int indexID;
                    float returnClass;

                    if ((findEndPosition = currentLine.find_first_of("label")) != string::npos)
                    {
                        nodeID = std::stoi(currentLine.substr(0, findEndPosition - 2));
                    }
                    if ((findStartPosition = currentLine.find("=\"[f")) != string::npos && (findEndPosition = currentLine.find("<")) != string::npos)
                    { // Verified there is no > character for Inner node
                        indexID = std::stoi(currentLine.substr(findStartPosition + 4, findEndPosition));
                    }
                    if ((findStartPosition = currentLine.find("<")) != string::npos && (findEndPosition = currentLine.find_first_of("]")) != string::npos)
                    { // Verified there is no > character for Inner node
                        returnClass = std::stod(currentLine.substr(findStartPosition + 1, findEndPosition));
                    }
                    tree.push_back(pdb::makeObject<pdb::Node>(nodeID, indexID, false, -1, -1, returnClass));
                }

		std::cout << "Processing leaf nodes" << std::endl;
                for (int i = 0; i < leafNodes.size(); ++i)
                { // Construct Leaf Nodes
                    string currentLine = leafNodes[i];
		    std::cout << currentLine << std::endl;
                    int nodeID;
                    float returnClass = -1.0f;
                    if ((findEndPosition = currentLine.find_first_of("label")) != string::npos)
                    {
                        nodeID = std::stoi(currentLine.substr(0, findEndPosition - 2));
                    }
                    // Output Class of XGBoost always a Double/Float. ProbabilityValue for Classification, ResultValue for Regression
                    if ((findStartPosition = currentLine.find("leaf")) != string::npos && (findEndPosition = currentLine.find("]")) != string::npos)
                    {
                        returnClass = std::stod(currentLine.substr(findStartPosition + 5, findEndPosition - 1));
                    }
                    tree.push_back(pdb::makeObject<pdb::Node>(nodeID, -1, true, -1, -1, returnClass));
                }

                std::cout << "Processing relationships" << std::endl;
                for (int i = 0; i < relationships.size(); ++i)
                { // Construct Directed Edges between Nodes
                    int parentNodeID;
                    int childNodeID;
                    std::string currentLine = relationships[i];
		    std::cout << currentLine << std::endl;
                    if ((findMidPosition = currentLine.find_first_of("->")) != std::string::npos)
                    {
                        parentNodeID = std::stoi(currentLine.substr(0, findMidPosition - 1));
                    }
                    if (parentNodeID == 0)
                    {
                        if ((findEndPosition = currentLine.find_first_of(" [")) != std::string::npos)
                        {
                            childNodeID = std::stoi(currentLine.substr(findMidPosition + 3, findEndPosition - 1 - (findMidPosition + 3)));
                        }
                    }
                    else
                    {
                        if ((findEndPosition = currentLine.find_first_of(" ;")) != std::string::npos)
                        {
                            childNodeID = std::stoi(currentLine.substr(findMidPosition + 3, findEndPosition - 1 - (findMidPosition + 3)));
                        }
                    }

                    for (int i = 0; i < tree.size(); ++i)
                    {
                        if (tree[i]->nodeID == parentNodeID)
                        {
                            if (tree[i]->leftChild == -1)
                            {
                                tree[i]->leftChild = childNodeID;
                            }
                            else
                            {
                                tree[i]->rightChild = childNodeID;
                            }
                        }
                    }
                }
                forest.push_back(tree);
		std::cout << "finished processing a tree" << std::endl;
            }

            // STATS ABOUT THE FOREST
            std::cout << "Number of trees in the forest: " << numTrees << std::endl;
            std::cout << "Number of nodes in each tree: " << std::endl;
            for (int i = 0; i < numTrees; i++)
            {
                std::cout << "Number of nodes in tree[" << i << "] is: " << forest[i].size() << std::endl;
            }
	
	}

        static bool compareByNodeID(const pdb::Node &a, const pdb::Node &b)
        {
            return a.nodeID < b.nodeID;
        }

        pdb::Vector<pdb::Vector<pdb::Handle<pdb::Node>>> & get_forest()
        {
            return forest;
        }

        void set_forest(pdb::Vector<pdb::Vector<pdb::Handle<pdb::Node>>>& forestIn)
        {
            forest = forestIn;
            numTrees = forestIn.size();
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
                //                case ModelType::LightGBM:
                //                    return 0.0f;
            default: // RF is default
                return most_common(begin, end);
            }
        }

        template <class T>
        pdb::Handle<pdb::Vector<T>> predict(Handle<TensorBlock2D<T>> &in)
        {                                 // TODO: Change all Double References to Float

            // get the input features matrix information
            uint32_t inNumRow = in->getRowNums();

            T *inData = in->getValue().rawData->c_ptr(); // Need to cast from Double to Float

            // set the output matrix
            pdb::Handle<pdb::Vector<T>> resultMatrix = pdb::makeObject<pdb::Vector<T>>(inNumRow);
            std::vector<T> thisResultMatrix(numTrees);


            T inputValue;

            for (int i = 0; i < inNumRow; i++)
            {
                for (int j = 0; j < numTrees; j++)
                {
                    //  inference
                    //  pass the root node of the tree
	            int curIndex = 0;
                    pdb::Node treeNode = *(forest[j][0]);
                    while (treeNode.isLeaf == false)
                    {
                        inputValue = inData[treeNode.indexID];
			std::cout << inputValue << "|" << treeNode.returnClass << ": 0->";

                        if (inputValue <= treeNode.returnClass)
                        {
			    curIndex = treeNode.leftChild;
			    std::cout << curIndex << "-L->";
                        }
                        else
                        {
			    curIndex = treeNode.rightChild;
			    std::cout << curIndex << "-R->";
                        }
			treeNode = *(forest[j][curIndex]);
                    }
                    thisResultMatrix[j] = treeNode.returnClass;
                }
                T voteResult = compute_result(thisResultMatrix.begin(), thisResultMatrix.end());
                (*resultMatrix)[i]=voteResult;
                std::cout << voteResult << std::endl;
	    }
            return resultMatrix;
        }
    };
}

#endif // NETSDB_FOREST_H
