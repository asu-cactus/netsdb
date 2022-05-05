// This is a test program for loading a tree from a local file
// To compile the code and run it, just : $g++ -std=c++11 Test_float.cc -o Test_float
// And then run the program using the following rules:
// The second parameter specifies whether to classification ("C") or regression ("R").
// Starting with the third parameter, each parameter will represent one path of a tree. The following is a running example.
// $./Test_float C ../higgs/higgs_0.txt ../higgs/higgs_1.txt
// $./Test_float C ../higgs/higgs_0.txt ../higgs/higgs_1.txt ../higgs/higgs_2.txt ../higgs/higgs_3.txt ../higgs/higgs_4.txt ../higgs/higgs_5.txt ../higgs/higgs_6.txt ../higgs/higgs_7.txt ../higgs/higgs_8.txt ../higgs/higgs_9.txt

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
class Node{

	public:

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

float most_common(float input_array[])
{
    int t;
    float most_value;
    int max = 1;
    for (int i = 0; i < (sizeof(input_array)/sizeof(input_array[0])); i++){
        t = 1;
        for (int j = 0; j < (sizeof(input_array)/sizeof(input_array[0])); j++){
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

int main(int argc, const char *argv[]){
	// the # of trees of the input
	int numTrees = argc-2;
	// "C" represents classification and "R" represents regression
	std::string type = std::string(argv[1]);

	// the forest to store all the trees
	std::vector<std::vector<Node>> forest;

	for(int n = 2; n < argc; n++){
		std::string inputFileName = std::string(argv[n]);
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
    	std::vector<Node> tree;

    	//std::cout << "print the innerNodes" << std::endl;
    	for (int i = 0; i < innerNodes.size(); i++) {
    		string thisLine = innerNodes[i];
    		int nodeID;
    		int indexID;
    		float returnClass;
    		//std::cout << innerNodes[i] << std::endl;
    		if((findEndPosition = thisLine.find_first_of("[label=")) != string::npos){
    			nodeID = std::stoi(thisLine.substr(0, findEndPosition-1));
    			//std::cout << nodeID << std::endl;
    		}
    		if((findStartPosition = thisLine.find("X")) != string::npos && (findEndPosition = thisLine.find_first_of("]")) != string::npos){
    			indexID = std::stoi(thisLine.substr(findStartPosition+2, findEndPosition-(findStartPosition+2)));
    			//std::cout << indexID << std::endl;
    		}
    		if((findStartPosition = thisLine.find("<=")) != string::npos && (findEndPosition = thisLine.find_first_of("ngini")) != string::npos){
    			returnClass = std::stod(thisLine.substr(findStartPosition+3, findEndPosition-1-(findStartPosition+3)));
    			//std::cout << returnClass << std::endl;
    		}
    		tree.push_back(Node(nodeID, indexID, false, -1, -1, returnClass));
    	}
    	//std::cout << "print the leafNodes" << std::endl;
    	for (int i = 0; i < leafNodes.size(); i++) {
    		string thisLine = leafNodes[i];
    		int nodeID;
    		float returnClass = -1.0;
    		//std::cout << leafNodes[i] << std::endl;
    		if((findEndPosition = thisLine.find_first_of("[label=\"gini")) != string::npos){
    			nodeID = std::stoi(thisLine.substr(0, findEndPosition-1));
    			//std::cout << nodeID << std::endl;
    		}
    		if(type == "C"){
    			if((findStartPosition = thisLine.find("nvalue = "))!= string::npos && (findMidPosition = thisLine.find_first_of(",")) != string::npos && (findEndPosition = thisLine.find_first_of("]\"] ;")) != string::npos){
    				float firstValue = std::stod(thisLine.substr(findStartPosition+10, findMidPosition-(findStartPosition+10)));
    				float secondValue = std::stod(thisLine.substr(findMidPosition+2, findEndPosition-(findMidPosition+2)));
    				//std::cout << firstValue << " " << secondValue << std::endl;
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
    		tree.push_back(Node(nodeID, -1, true, -1, -1, returnClass));
    	}
    	//std::cout << "number of tree nodes: " << tree.size() << std::endl;
    	//std::cout << "print the relationships" << std::endl;
    	for (int i = 0; i < relationships.size(); i++) {
    		//std::cout << relationships[i] << std::endl;
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
    		std::vector<Node>::iterator iter;
    		iter = find(tree.begin(), tree.end(), nodeID);
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
    	forest.push_back(tree);
    }
    
    std::cout << "numbers of trees in the forest: " << forest.size() << std::endl;
    std::cout << "numbers of nodes in each tree: " << std::endl;
    for(int i = 0; i < forest.size(); i++){
    	std::cout << "numbers of nodes in tree " << i << " is " << forest[i].size() << std::endl;
    }

    // set the input array size
    int inNumRow = 10000;
    int inNumFeature = 28;

    // randomly generate the inputs
    float input[inNumRow][inNumFeature];
    // seed
    srand(time(NULL));
    for(int i = 0; i < inNumRow; i++){
		for(int j = 0; j < inNumFeature; j++){
            input[i][j] = rand() % (N + 1) / (float)(N + 1);
        }
    }

    // set the output array before voting
    float thisResultMatrix[inNumRow][numTrees];

    // set the final output array
    float resultMatrix[inNumRow];

    float inputValue;
    Node * treeNode = nullptr;
    // do the inference
    auto begin = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < inNumRow; i++){
        for(int j = 0; j < numTrees; j++){
            // pass the root node of the tree
            treeNode = & forest[j][0];
            while(treeNode->isLeaf == false){
                inputValue = input[i][treeNode->indexID];
                if(inputValue <= treeNode->returnClass){
                    * treeNode = * (treeNode + (treeNode->leftChild));
                }else{
                    * treeNode = * (treeNode + (treeNode->rightChild));
                }
            }
            thisResultMatrix[i][j] = treeNode->returnClass;
        }
        float voteResult = most_common(thisResultMatrix[i]);
        resultMatrix[i] = voteResult;
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    std::cout << "Float in C Inference Time: " << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count() << " secs." << std::endl;
    return 0;
    /*
    for (int i = 0; i < inNumRow; i++){
        std::cout << resultMatrix[i] << endl;
    }
    */
}