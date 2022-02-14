#ifndef TREE_H
#define TREE_H

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <cstring>

#include "FFMatrixBlockScanner.h"
#include "FFTransposeMult.h"
#include "FFAggMatrix.h"
#include "FFMatrixWriter.h"
#include "FFMatrixBlock.h"
#include "TreeNode.h"
#include "PDBClient.h"
#include "StorageClient.h"

// this file describes a decision tree

using namespace pdb;

namespace decisiontree{

	class Tree: public Object {

	public:

		ENABLE_DEEP_COPY

		pdb::Vector<pdb::Handle<decisiontree::Node>> tree;

		Tree() {}
		~Tree() {}

		Tree(pdb::Vector<pdb::Handle<decisiontree::Node>> treeIn){
			tree = treeIn;
		}

		pdb::Vector<pdb::Handle<decisiontree::Node>> get_tree() {
			return tree;
		}

		void set_tree(pdb::Vector<pdb::Handle<decisiontree::Node>> treeIn) {
			tree = treeIn;
		}

		pdb::Handle<pdb::Vector<double>> predict(Handle<FFMatrixBlock>& in){

			// get the input features matrix information
			uint32_t inNumRow = in->getRowNums();
			uint32_t inNumCol = in->getColNums();

			double *inData = in->getValue().rawData->c_ptr();

			// set the output matrix
			pdb::Handle<pdb::Vector<double>> resultMatrix = pdb::makeObject<pdb::Vector<double>>();

			// set the node of the tree
			decisiontree::Node * treeNode = nullptr;

			// set a new vetor to store the whole tree
			std::vector<decisiontree::Node> vectNode;
			for(int i = 0; i < tree.size(); i++){
				pdb::Handle<decisiontree::Node> thisNodePtr = tree[i];
				decisiontree::Node thisNode = decisiontree::Node(thisNodePtr->nodeID,thisNodePtr->indexID,thisNodePtr->isLeaf,thisNodePtr->leftChild,thisNodePtr->rightChild,thisNodePtr->returnClass);
				vectNode.push_back(thisNode);
			}

			// inference
			double inputValue;
			pdb::Handle<decisiontree::Node> thisNodePtr;
			for (int i = 0; i < inNumRow; i++){
				// pass the root node of the tree
				treeNode = & vectNode.at(0);
				while(treeNode->isLeaf == false){
					inputValue = inData[i*inNumCol+treeNode->indexID];
					if(inputValue <= treeNode->returnClass){
						* treeNode = * (treeNode + (treeNode->leftChild));
					}else{
						* treeNode = * (treeNode + (treeNode->rightChild));
					}
				}
				std::cout << treeNode->returnClass << " ";
				resultMatrix->push_back(treeNode->returnClass);
			}
			return resultMatrix;
		}
	};
}

#endif