
#ifndef NETSDB_TREE_H
#define NETSDB_TREE_H

#define MAX_NUM_NODES_PER_TREE 512

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

        Node tree[MAX_NUM_NODES_PER_TREE];

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
             for (int i = 0; i < MAX_NUM_NODES_PER_TREE; i++) {
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


        static void processInnerNodes(std::vector<std::string> & innerNodes, ModelType modelType, Node * tree)
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
                        indexID = std::stoi(currentLine.substr(findStartPosition + 2, findEndPosition-findStartPosition-2));
                    }

                   if ((findStartPosition = currentLine.find("<=")) != string::npos && (findEndPosition = currentLine.find_first_of("\\ngini")) != string::npos)
                    {
                        returnClass = std::stod(currentLine.substr(findStartPosition + 3, findEndPosition-findStartPosition-3));

                    }
               } else if (modelType == ModelType::XGBoost) {
                   if ((findEndPosition = currentLine.find_first_of("[ label")) != string::npos) {
                       nodeID = std::stoi(currentLine.substr(4, findEndPosition - 1-4));
                   }
                   if ((findStartPosition = currentLine.find("f")) != string::npos && (findEndPosition = currentLine.find("<")) != string::npos)
                   {
                        indexID = std::stoi(currentLine.substr(findStartPosition + 1, findEndPosition-findStartPosition-1));
                    }
                   if ((findStartPosition = currentLine.find("<")) != string::npos && (findEndPosition = currentLine.find_first_of("]")) != string::npos)
                    {
                            returnClass = std::stod(currentLine.substr(findStartPosition + 1, findEndPosition-findStartPosition-1));
                    }
               } else {
	       
	            if (((findStartPosition = currentLine.find_first_of("split")) != string::npos)&&((findEndPosition = currentLine.find_first_of("[label=<<B>")) != string::npos)) {
                       nodeID = std::stoi(currentLine.substr(findStartPosition+5, findEndPosition - 1-findStartPosition-5));
                    }
                    if ((findStartPosition = currentLine.find("Column_")) != string::npos && (findEndPosition = currentLine.find("</B> &#8804;")) != string::npos) {
                        indexID = std::stoi(currentLine.substr(findStartPosition+7, findEndPosition-findStartPosition-7));
                    }
                    if ((findStartPosition = currentLine.find("&#8804;<B>")) != string::npos && (findEndPosition = currentLine.find_first_of("</B>> fillcolor=")) != string::npos) {
                        returnClass = std::stod(currentLine.substr(findStartPosition + 10, findEndPosition-findStartPosition-10));
                    }

	       }
               tree[nodeID].indexID = indexID;
               tree[nodeID].isLeaf = false;
               tree[nodeID].leftChild = -1;
               tree[nodeID].rightChild = -1;
               tree[nodeID].returnClass = returnClass;
	       
           }

        }

        static void processLeafNodes(std::vector<std::string> & leafNodes, ModelType modelType, Node * tree)
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
                        returnClass = std::stod(currentLine.substr(findStartPosition + 2, findEndPosition-findStartPosition-2));
                    }
                } else if (modelType == ModelType::XGBoost) {

                    if ((findEndPosition = currentLine.find_first_of("[")) != string::npos)
                    {
                        nodeID = std::stoi(currentLine.substr(4, findEndPosition-1-4));
                    }
                    // Output Class of XGBoost always a Double/Float. ProbabilityValue for Classification, ResultValue for Regression
                    if ((findStartPosition = currentLine.find("leaf=")) != string::npos && (findEndPosition = currentLine.find("]")) != string::npos)
                    {
                        returnClass = std::stod(currentLine.substr(findStartPosition+5, findEndPosition-3-findStartPosition-5));
                    }

                } else {
		
		    if (((findStartPosition = currentLine.find_first_of("leaf")) != string::npos)&&((findEndPosition = currentLine.find_first_of("[")) != string::npos))
                    {
                        nodeID = std::stoi(currentLine.substr(findStartPosition+4, findEndPosition-1-findStartPosition-4));
                    }
                    // Output Class of XGBoost always a Double/Float. ProbabilityValue for Classification, ResultValue for Regression
                    if ((findStartPosition = currentLine.find(": <B>")) != string::npos && (findEndPosition = currentLine.find("</B>>]")) != string::npos)
                    {
                        returnClass = std::stod(currentLine.substr(findStartPosition+5, findEndPosition-findStartPosition-5));
			
                    }
		
		}
		if (modelType == ModelType::LightGBM) {
		    nodeID = nodeID + MAX_NUM_NODES_PER_TREE/2;
		}
                tree[nodeID].indexID = -1;
                tree[nodeID].isLeaf = true;
                tree[nodeID].leftChild = -1;
                tree[nodeID].rightChild = -1;
                tree[nodeID].returnClass = returnClass;
            }
        }

	static void processRelationships(std::vector<std::string> & relationships, ModelType modelType, Node * tree)
        {


                int findStartPosition;
                int findMidPosition;
                int findEndPosition;

                for (int i = 0; i < relationships.size(); ++i)
                {
		    // Construct Directed Edges between Nodes
                    int parentNodeID;
                    int childNodeID;
                    std::string currentLine = relationships[i];
		    bool isChildLeaf;

                    if (modelType == ModelType::RandomForest) {
                        if ((findMidPosition = currentLine.find_first_of("->")) != std::string::npos)
                        {
                            parentNodeID = std::stoi(currentLine.substr(0, findMidPosition-1));
                        }
                        if (parentNodeID == 0) {

                            if ((findEndPosition = currentLine.find_first_of("[label")) != std::string::npos) {

                                childNodeID = std::stoi(currentLine.substr(findMidPosition + 3, findEndPosition - 1-findMidPosition-3));

                            }
                        } else {

                            if ((findEndPosition = currentLine.find_first_of(";")) != std::string::npos) {

                                childNodeID = std::stoi(currentLine.substr(findMidPosition + 3, findEndPosition-1-findMidPosition-3));

                            }

                        }

                    } else if (modelType == ModelType::XGBoost) {

                        if ((findMidPosition = currentLine.find_first_of("->")) != std::string::npos) {

                            parentNodeID = std::stoi(currentLine.substr(4, findMidPosition - 1-4));
                        }
                        if ((findEndPosition = currentLine.find_first_of("[")) != std::string::npos) {

                                childNodeID = std::stoi(currentLine.substr(findMidPosition + 3, findEndPosition-1-findMidPosition-3));
                        }

                    } else {
		    
		        if (((findStartPosition = currentLine.find_first_of("split")) != std::string::npos)&&((findMidPosition = currentLine.find_first_of("->")) != std::string::npos)) {

                            parentNodeID = std::stoi(currentLine.substr(findStartPosition+5, findMidPosition - 1-findStartPosition-5));
                        }
                        if ((findEndPosition = currentLine.find("[")) != std::string::npos) {
                                if (currentLine.find("leaf") != std::string::npos) {
                                    childNodeID = std::stoi(currentLine.substr(findMidPosition + 7, findEndPosition-1-findMidPosition-7));
				    childNodeID = childNodeID + MAX_NUM_NODES_PER_TREE/2;
				} else {
			            childNodeID = std::stoi(currentLine.substr(findMidPosition + 8, findEndPosition-1-findMidPosition-8));	
			        }
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
                    if ((line.size()==0) || (line.find("graph")!=std::string::npos) || (line.find("digraph Tree {")!=std::string::npos) || (line.find("node [shape=box")!=std::string::npos) || (line.find("edge [fontname=")!=std::string::npos) || (line.find("}")!=std::string::npos))
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

                processInnerNodes(innerNodes, modelType, this->tree);
                processLeafNodes(leafNodes, modelType, this->tree);
		processRelationships(relationships, modelType, this->tree);

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
