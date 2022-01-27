#ifndef BASELINE_NODE_H
#define BASELINE_NODE_H

// this file describes a baseline decision tree node

#include <vector>

using namespace pdb;

namespace decisiontree{

	class GenericNode: public Object {

	private:
		int nodeID = 0;
		int indexID = 0;
		bool isLeaf = false;
		GenericNode * leftChild = nullptr;
		GenericNode * rightChild = nullptr;
		// returnClass will be the vaule to compare while this is not a leaf node
		double returnClass = 0L;
		
	public:

		ENABLE_DEEP_COPY

		GenericNode() {}
		~GenericNode() {}

		GenericNode(int nodeIDIn, int indexIDIn, bool isLeafIn, GenericNode & leftChildIn, GenericNode & rightChildIn, double returnClassIn){
			nodeID = nodeIDIn;
			indexID = indexIDIn;
			isLeaf = isLeafIn;
			leftChild = & leftChildIn;
			rightChild = & rightChildIn;
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

		GenericNode * get_leftChild() {
			return leftChild;
		}

		GenericNode * get_rightChild() {
			return rightChild;
		}

		double get_returnClass() {
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

		void set_leftChild(GenericNode & leftChildIn) {
			leftChild = & leftChildIn;
		}

		void set_rightChild(GenericNode & rightChildIn) {
			rightChild = & rightChildIn;
		}

		void set_returnClass(double returnClassIn) {
			returnClass = returnClassIn;
		}

		double predict(std::vector<double> inputFeatures){
			if(isLeaf){
				return returnClass;
			}else{
				if(inputFeatures.at(indexID) < returnClass){
					return leftChild->predict(inputFeatures);
				}else{
					return rightChild->predict(inputFeatures);
				}
			}
		}
	};
}

#endif