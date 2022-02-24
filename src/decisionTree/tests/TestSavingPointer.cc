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
#include <string.h>

using namespace std;

static double LONG_DEFAULT = 0.0;
static int INTEGER_DEFAULT = 0;
static bool BOOL_DEFAULT = false;
static int NUM_INT = 4;
static int NUM_LONG = 1;
static int NUM_BOOL = 1;
static int NUM_NODE = 10000;

// this is the tree node class, which exactly the same as class Node in TreeNode.h under /src/decisionTree/headers/TreeNode.h
class Node{

	public:

		int nodeID = INTEGER_DEFAULT;
		int indexID = INTEGER_DEFAULT;
		bool isLeaf = BOOL_DEFAULT;
		int leftChild = INTEGER_DEFAULT;
		int rightChild = INTEGER_DEFAULT;
		// returnClass will be the vaule to compare while this is not a leaf node
		double returnClass = LONG_DEFAULT;

		Node() {}
		~Node() {}

		Node(int nodeIDIn, int indexIDIn, bool isLeafIn, int leftChildIn, int rightChildIn, double returnClassIn){
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

		void set_leftChild(int leftChildIn) {
			leftChild = leftChildIn;
		}

		void set_rightChild(int rightChildIn) {
			rightChild = rightChildIn;
		}

		void set_returnClass(double returnClassIn) {
			returnClass = returnClassIn;
		}
};

class RandomForest {

	public:

		std::vector<std::vector<Node>> forest;
		int numTree;

		RandomForest() {}
		~RandomForest() {}

		RandomForest(std::vector<std::vector<Node>> forestIn){
			forest = forestIn;
			numTree = forestIn.size();
		}

		std::vector<std::vector<Node>> get_forest() {
			return forest;
		}

		void set_forest(std::vector<std::vector<Node>> forestIn) {
			forest = forestIn;
			numTree = forestIn.size();
		}
	};

int main(int argc, const char *argv[]){

	/*
	int size = (NUM_INT * sizeof(int) + NUM_LONG * sizeof(double) + NUM_BOOL * sizeof(bool)) * NUM_NODE;

	Node* tree = static_cast<Node*>(mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, 0, 0));

	*tree = Node(0,0,false,1,2,-1.00);
	*(tree + 1) = Node(1,1,true,-1,-1,2.00);
	*(tree + 2) = Node(2,2,true,-1,-1,1.00);

	//print the original root address
	std::cout << "original root address: " << tree << std::endl;

	//string fileName = "/home/jiaqingchen/netsdb/trees/tree";
	string fileName = "tree";
	ofstream file(fileName);
	if (file){
		file << tree << "\n";
	}

	file.close();

	// reload the pointer from tree.csv file
	ifstream fin(fileName);
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
	*/

	/*
	int size = (NUM_INT * sizeof(int) + NUM_LONG * sizeof(double) + NUM_BOOL * sizeof(bool)) * NUM_NODE;

	Node* tree = static_cast<Node*>(mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, 0, 0));

	std::vector<Node> treeNodes;
	for(int i = 0; i < NUM_NODE; i++){
		treeNodes.push_back(Node(0,0,false,1,2,-1.00));
	}

	for(int i = 0; i < treeNodes.size(); i++){
		*(tree + i) = treeNodes.at(i);
	}

	//print the original root address
	std::cout << "original root address: " << tree << std::endl;

	//std::cout << "start saving the random forest information" << std::endl;
	std::vector<int> info;
	info.push_back(1);
	int numTrees = 1;
	//int infomemSize = std::max(static_cast<int>(numTrees * sizeof(int)),20);
	int infomemSize = numTrees * sizeof(int);
	int * infoPtr = static_cast<int*>(mmap(NULL, infomemSize, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, 0, 0));
	for(int t = 0; t < numTrees; t++){
		*(infoPtr + t) = info.at(t);
	}
	std::cout << "original info address: " << infoPtr << std::endl;

	//string fileName = "/home/jiaqingchen/netsdb/trees/tree";
	string fileName = "tree";
	ofstream file(fileName);
	if (file){
		file << tree << "\n";
		file << infoPtr << "\n";
	}

	file.close();

	// reload the pointer from tree.csv file
	ifstream fin(fileName);
	string line;
	Node * ptr = nullptr;
	int * inPtr = nullptr;
	int count = 0;
	while (getline(fin, line)){
		if(count == 0){
			// print the string obtained from csv file
			//std::cout << "string obtained from csv file: " << line << std::endl;
			long long result=strtoll(line.c_str(), NULL, 16);
			ptr = (Node *)result;
			// print the new address we get from the file
			std::cout << "new root address: " << ptr << std::endl;
			count++;
		}else{
			long long result=strtoll(line.c_str(), NULL, 16);
			inPtr = (int *)result;
			// print the new address we get from the file
			std::cout << "new info address: " << inPtr << std::endl;
		}	
	}
	// print some values of the tree nodes
	std::cout << "root node's nodeID: " << ptr-> nodeID << std::endl;
	std::cout << "root's left child node's data: " << (ptr+(ptr->leftChild))-> returnClass << std::endl;
	munmap(tree, size);
	munmap(infoPtr, infomemSize);

	*/

	/*
	int size = (NUM_INT * sizeof(int) + NUM_LONG * sizeof(double) + NUM_BOOL * sizeof(bool)) * NUM_NODE;

	std::vector<Node> treeNodes;
	for(int i = 0; i < NUM_NODE; i++){
		treeNodes.push_back(Node(0,0,false,1,2,-1.00));
	}

	std::vector<int> info;
	int numTrees = 1;
	info.push_back(numTrees);
	info.push_back(NUM_NODE);
	int infomemSize = (numTrees+1) * sizeof(int);

	int totalSize = size + infomemSize;

	std::cout << "int value size is " << infomemSize << std::endl;
	std::cout << "node value size is " << size << std::endl;
	std::cout << "total size is " << totalSize << std::endl;

	void* startPtr = mmap(NULL, totalSize, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, 0, 0);
	void* orgPtr = startPtr;

	//print the original address
	std::cout << "original address: " << startPtr << std::endl;

	int * intPtr = (int *)startPtr;
	for(int t = 0; t < info.size(); t++){
		std::cout << "int value writing is " << info.at(t) << std::endl;
		//*(intPtr + t) = info.at(t);
		memcpy((intPtr + t),&info.at(t),sizeof(int));
	}
	intPtr = intPtr + info.size();

	startPtr = intPtr;
	std::cout << "the start pointer address of nodes is " << startPtr << std::endl;
	Node * nodePtr = (Node *)startPtr;

	for(int i = 0; i < treeNodes.size(); i++){
		*(nodePtr + i) = treeNodes.at(i);
	}

	string fileName = "tree";
	ofstream file(fileName);
	if (file){
		file << orgPtr << "\n";
	}

	file.close();

	// reload the pointer from tree file
	ifstream fin(fileName);
	string line;
	Node * newNodePtr = nullptr;
	int * newIntPtr = nullptr;
	void * newPtr = nullptr;

	while (getline(fin, line)){
		// print the string obtained from csv file
		//std::cout << "string obtained from csv file: " << line << std::endl;
		long long result=strtoll(line.c_str(), NULL, 16);
		newPtr = (void *)result;
		// print the new address we get from the file
		std::cout << "new address: " << newPtr << std::endl;
	}

	std::cout << "start position int value is " << *(int *)newPtr << std::endl;
	newIntPtr = (int*)newPtr;
	int numTree = * newIntPtr;
	std::cout << "numTree value is " << numTree << std::endl;
	std::vector<int> numRoots;
	for(int i = 0; i < numTree; i++){
		std::cout << "the number of nodes in this tree is " << *(newIntPtr+i+1) << std::endl;
		numRoots.push_back(*(newIntPtr+i+1));
	}

	newPtr = newIntPtr + numTree + 1;
	newNodePtr = (Node *)newPtr;
	std::cout << "start address of nodes is " << newNodePtr << std::endl;

	// print some values of the tree nodes
	std::cout << "number of trees is " << numRoots.size() << std::endl;
	std::cout << "one root node return class is " << newNodePtr->returnClass << std::endl;

	munmap(startPtr, totalSize);
	*/

	int size = (NUM_INT * sizeof(int) + NUM_LONG * sizeof(double) + NUM_BOOL * sizeof(bool)) * NUM_NODE;

	std::vector<Node> treeNodes;
	for(int i = 0; i < NUM_NODE; i++){
		treeNodes.push_back(Node(0,0,false,1,2,-1.00));
	}

	std::vector<std::vector<Node>> thisForest;
	thisForest.push_back(treeNodes);

	int totalSize = size + 4;

	std::cout << "total size is " << totalSize << std::endl;

	RandomForest * startPtr = static_cast<RandomForest*>(mmap(NULL, totalSize, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, 0, 0));

	//print the original address
	std::cout << "original address: " << startPtr << std::endl;

	* startPtr = RandomForest(thisForest);

	string fileName = "tree";
	ofstream file(fileName);
	if (file){
		file << startPtr << "\n";
	}

	file.close();

	// reload the pointer from tree file
	ifstream fin(fileName);
	string line;
	RandomForest * newPtr = nullptr;

	while (getline(fin, line)){
		// print the string obtained from file
		//std::cout << "string obtained from file: " << line << std::endl;
		long long result=strtoll(line.c_str(), NULL, 16);
		newPtr = (RandomForest*)result;
		// print the new address we get from the file
		std::cout << "new address: " << newPtr << std::endl;
	}

	// print some values of the loading forest
	std::cout << "number of trees is " << newPtr->numTree << std::endl;
	std::vector<Node> tree = newPtr->forest.at(0);
	std::cout << "number of nodes is " << tree.size() << std::endl;
	Node node = tree.at(0);
	std::cout << "one root node return class is " << node.returnClass << std::endl;

	munmap(startPtr, totalSize);

	return 0;
}