// This is a test program for saving a pointer address into a csv file and reload the file to get the pointer.
// To compile the code and run it, just : $g++ -std=c++11 TestSavingPointer.cc -o TestSavingPointer
// And then run the program: $./TestSavingPointer

#include <iostream>
#include <fstream>
#include <future>
#include <thread>
#include <sstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cassert>
#include <memory>
#include <vector>
#include <algorithm>
#include <map>
#include <set>
#include <cstring>
#include <exception>

using namespace std;

static long LONG_DEFAULT = 0L;
static int INTEGER_DEFAULT = 0;
static bool BOOL_DEFAULT = false;
static int NUM_INT = 4;
static int NUM_LONG = 1;
static int NUM_BOOL = 1;
static int NUM_NODE = 3;

// this is the tree node class, which exactly the same as class Node in TreeNode.h under /src/decisionTree/headers/TreeNode.h
class Node{

	public:

		int nodeID = INTEGER_DEFAULT;
		int indexID = INTEGER_DEFAULT;
		bool isLeaf = BOOL_DEFAULT;
		int leftChild = INTEGER_DEFAULT;
		int rightChild = INTEGER_DEFAULT;
		// returnClass will be the vaule to compare while this is not a leaf node
		long returnClass = LONG_DEFAULT;

		Node() {}
		~Node() {}

		Node(int nodeIDIn, int indexIDIn, bool isLeafIn, int leftChildIn, int rightChildIn, long returnClassIn){
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

		long get_returnClass() {
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

		void set_returnClass(long returnClassIn) {
			returnClass = returnClassIn;
		}
};

int main(int argc, const char *argv[]){

	int size = (NUM_INT * sizeof(int) + NUM_LONG * sizeof(long) + NUM_BOOL * sizeof(bool)) * NUM_NODE;

	Node* tree = static_cast<Node*>(mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, 0, 0));

	*tree = Node(0,0,false,1,2,-1.00);
	*(tree + 1) = Node(1,1,true,-1,-1,2.00);
	*(tree + 2) = Node(2,2,true,-1,-1,1.00);

	//print the original root address
	std::cout << "original root address: " << tree << std::endl;

	ofstream file("tree.csv");
	if (file){
		file << tree << "\n";
	}

	file.close();

	// reload the pointer from tree.csv file
	ifstream fin("tree.csv");
	string line;
	Node * ptr = nullptr;
	while (getline(fin, line)){
		// print the string obtained from csv file
		std::cout << "string obtained from csv file: " << line << std::endl;
		long long result=strtoll(line.c_str(), NULL, 16);
		ptr = (Node *)result;
		// print the new address we get from the file
		std::cout << "new root address: " << ptr << std::endl;
	}
	// print some values of the tree nodes
	std::cout << "root node's nodeID: " << ptr-> nodeID << std::endl;
	std::cout << "root's left child node's data: " << (ptr+(ptr->leftChild))-> returnClass << std::endl;
	munmap(tree, size);
	return 0;
}