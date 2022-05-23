#ifndef TREE_NODE_H
#define TREE_NODE_H

// this file describes a decision tree node
// each set in the database is a tree
// each row in one set is a decision tree node


// PRELOAD %Node%

using namespace pdb;

static float FLOAT_DEFAULT = 0.0;
static int INTEGER_DEFAULT = 0;
static bool BOOL_DEFAULT = false;

namespace pdb{

	class Node: public Object {

	public:

		ENABLE_DEEP_COPY

		int nodeID = INTEGER_DEFAULT;
		int indexID = INTEGER_DEFAULT;
		bool isLeaf = BOOL_DEFAULT;
		int leftChild = INTEGER_DEFAULT;
		int rightChild = INTEGER_DEFAULT;
		// returnClass will be the vaule to compare while this is not a leaf node
		float returnClass = FLOAT_DEFAULT;

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

		float get_returnClass() {
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
}

#endif
