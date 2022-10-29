
#ifndef NETSDB_TREE_H
#define NETSDB_TREE_H

#define MAX_TREE_NODES 512

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
#include "TreeResult.h"

// PRELOAD %Tree%

using std::filesystem::directory_iterator;

namespace pdb
{

    class Tree : public Object
    {
    public:
        //ENABLE_DEEP_COPY

        Node tree[MAX_TREE_NODES];

        Tree() {}


        int treeId;

	ModelType modelType;

	Tree(int treeId, std::string treePath, ModelType modelType) 
	{
	    this->treeId = treeId;
	    this->modelType = modelType;
	    this->constructTreeFromPath(treePath, modelType);
	
	}


        void setUpAndCopyFrom(void* target, void* source) const override {
             new (target) Tree ();
             Tree& fromMe = *((Tree *) source);
             Tree& toMe = *((Tree *) target);
             for (int i = 0; i < MAX_TREE_NODES; i++) {
	         toMe.tree[i].indexID = fromMe.tree[i].indexID;
                 toMe.tree[i].isLeaf = fromMe.tree[i].isLeaf;
		 toMe.tree[i].leftChild = fromMe.tree[i].leftChild;
		 toMe.tree[i].rightChild = fromMe.tree[i].rightChild;
		 toMe.tree[i].returnClass = fromMe.tree[i].returnClass;
	     }
	     toMe.treeId = fromMe.treeId;
	     toMe.modelType = fromMe.modelType;

	}

         void deleteObject(void* deleteMe) override {
             deleter(deleteMe, this);
         }

         size_t getSize(void* forMe) override {
             return sizeof(Tree);
         }

        void processInnerNodes(std::vector<std::string> & innerNodes, ModelType modelType)
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
               tree[nodeID].indexID = indexID;
               tree[nodeID].isLeaf = false;
               tree[nodeID].leftChild = -1;
               tree[nodeID].rightChild = -1;
               tree[nodeID].returnClass = returnClass;
           }

        }

        void processLeafNodes(std::vector<std::string> & leafNodes, ModelType modelType)
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
                tree[nodeID].indexID = -1;
                tree[nodeID].isLeaf = true;
                tree[nodeID].leftChild = -1;
                tree[nodeID].rightChild = -1;
                tree[nodeID].returnClass = returnClass;
            }
        }

	void processRelationships(std::vector<std::string> & relationships, ModelType modelType)
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

                    if (tree[parentNodeID].leftChild == -1)
                    {
                        tree[parentNodeID].leftChild = childNodeID;
                    }
                    else
                    {
                        tree[parentNodeID].rightChild = childNodeID;
                    }

                }

	}          

        void constructTreeFromPath (std::string & treePathIn, ModelType modelType) {

                std::string inputFileName = treePathIn;
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

                processInnerNodes(innerNodes, modelType);
                processLeafNodes(leafNodes, modelType);
		processRelationships(relationships, modelType);

	}


        pdb::Handle<TreeResult> predict(Handle<TensorBlock2D<float>> &in)
        {                                 // TODO: Change all Double References to Float

            // get the input features matrix information
            uint32_t rowIndex = in->getBlockRowIndex();
	    int numRows = in->getRowNums();
	    int numCols = in->getColNums(); 
            float *inData = in->getValue().rawData->c_ptr();

            // set the output matrix
            pdb::Handle<TreeResult> resultMatrix = pdb::makeObject<TreeResult>(treeId, rowIndex, numRows, modelType);

	    float * outData = resultMatrix->data;

	    int featureStartIndex;

            for (int i = 0; i < numRows; i++)
            {
		    featureStartIndex = i*numCols;
                    int curIndex = 0;
                    while (tree[curIndex].isLeaf == false)
                    {
                        if (inData[featureStartIndex + tree[curIndex].indexID] <= tree[curIndex].returnClass)
                        {
			    curIndex = tree[curIndex].leftChild;
                        }
                        else
                        {
			    curIndex = tree[curIndex].rightChild;
                        }
                    }

                    outData[i] = (float)(tree[curIndex].returnClass);
	    }

            return resultMatrix;
        }


    };
}

#endif // NETSDB_TREE_H
