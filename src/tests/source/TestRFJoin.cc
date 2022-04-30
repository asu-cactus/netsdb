#ifndef TEST_RF_JOIN_CC
#define TEST_RF_JOIN_CC

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstring>
#include <algorithm>
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
#include <map>
#include <set>
#include <cstdio>
#include <exception>
#include <cassert>

#include "PDBClient.h"
#include "StorageClient.h"
#include "PDBClient.h"
#include "BaselineNode.h"
#include "MyPrediction.h"
#include "TreeNode.h"
#include "Tree.h"
#include "RandomForest.h"
#include "FFMatrixBlockScanner.h"
#include "FFTransposeMult.h"
#include "FFAggMatrix.h"
#include "FFMatrixWriter.h"
#include "FFMatrixBlock.h"
#include "FFMatrixUtil.h"
#include "SimpleFF.h"
#include "RFJoin.h"

using namespace pdb;

// 1st parameter is the program itself
// 2nd parameter gives whether to create set or not
// 3rd - 6th parameter gives the number of rowNum, colNum, block_x and block_y
// testing for higgs, an example for 3rd - 6th parameters are: 2000,28,100,28
// 7th parameter specifies whether to classification ("C") or regression ("R")
// Starting with the 8th parameter, each parameter will represent one path of a tree. The following is a running example.
// $bin/rfJoin Y 2000 28 100 28 C /home/jiaqingchen/netsdb/graphs/higgs/higgs_0.txt /home/jiaqingchen/netsdb/graphs/higgs/higgs_1.txt
// $bin/rfJoin Y 2000 90 100 90 R /home/jiaqingchen/netsdb/graphs/year/year_0.txt /home/jiaqingchen/netsdb/graphs/year/year_1.txt
// $bin/rfJoin Y 11000 28 1000 28 C /home/jiaqingchen/netsdb/graphs/higgs/higgs_0.txt /home/jiaqingchen/netsdb/graphs/higgs/higgs_1.txt /home/jiaqingchen/netsdb/graphs/higgs/higgs_2.txt /home/jiaqingchen/netsdb/graphs/higgs/higgs_3.txt /home/jiaqingchen/netsdb/graphs/higgs/higgs_4.txt /home/jiaqingchen/netsdb/graphs/higgs/higgs_5.txt /home/jiaqingchen/netsdb/graphs/higgs/higgs_6.txt /home/jiaqingchen/netsdb/graphs/higgs/higgs_7.txt /home/jiaqingchen/netsdb/graphs/higgs/higgs_8.txt /home/jiaqingchen/netsdb/graphs/higgs/higgs_9.txt
int main(int argc, char *argv[]) {

    makeObjectAllocatorBlock(1024 * 1024 * 1024, true);

    bool createSetOrNot = true;

    if ( argc > 1 ) {
        if ( strcmp( argv[1], "Y") == 0 ) {
            createSetOrNot = true;
        } else {
            createSetOrNot = false;
        }
    }

	int rowNum = std::stoi(argv[2]);
	int colNum = std::stoi(argv[3]);
	int block_x = std::stoi(argv[4]);
	int block_y = std::stoi(argv[5]);

	string errMsg;

	string masterIp = "localhost";
	pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("clientLog");
	pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
	pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);

    if(createSetOrNot == true){
        pdbClient.registerType("libraries/libTreeNode.so", errMsg);
        pdbClient.registerType("libraries/libTree.so", errMsg);
        pdbClient.registerType("libraries/libRandomForest.so", errMsg);
        pdbClient.registerType("libraries/libRFJoin.so", errMsg);
        pdbClient.registerType("libraries/libFFMatrixMeta.so", errMsg);
        pdbClient.registerType("libraries/libFFMatrixData.so", errMsg);
        pdbClient.registerType("libraries/libFFMatrixBlock.so", errMsg);
        pdbClient.registerType("libraries/libFFMatrixBlockScanner.so", errMsg);
        pdbClient.registerType("libraries/libFFMatrixWriter.so", errMsg);
        pdbClient.registerType("libraries/libFFMatrixPartitioner.so", errMsg);
    } else{
        std::cout << "Not load shared libraries" << std::endl;
    }
    
    string dbName = "rf_db";
    string setName = "rf_set";

    if(createSetOrNot == true){
        // create a new database
        if (!pdbClient.createDatabase(dbName, errMsg)) {
            cout << "Not able to create database: " + errMsg;
            return -1;
        } else {
            std::cout << "Created Random Forest database" << std::endl;
        }

        // create a new set in the database
        if (!pdbClient.createSet<decisiontree::RandomForest>(dbName, setName, errMsg)) {
            cout << "Not able to create set: " + errMsg;
            return -1;
        } else {
            std::cout << "Created Random Forest set" << std::endl;
        }

        // the # of trees of the input
        int numTrees = argc-7;
        // "C" represents classification and "R" represents regression
        std::string type = std::string(argv[6]);

        pdb::Vector<pdb::Vector<pdb::Handle<decisiontree::Node>>> forest;
        pdb::Handle<pdb::Vector<pdb::Handle<decisiontree::RandomForest>>> storeMe = pdb::makeObject<pdb::Vector<pdb::Handle<decisiontree::RandomForest>>>();

        for(int n = 7; n < argc; n++){
            std::string inputFileName = std::string(argv[n]);
            std::cout << "tree file path: " << inputFileName << std::endl;
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

            while(getline(infile,line)){
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
                    } else{
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
            pdb::Vector<pdb::Handle<decisiontree::Node>> tree;

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
                tree.push_back(pdb::makeObject<decisiontree::Node>(nodeID, indexID, false, -1, -1, returnClass));
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
                tree.push_back(pdb::makeObject<decisiontree::Node>(nodeID, -1, true, -1, -1, returnClass));
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

                for(int k = 0; k < tree.size(); k++){
                    pdb::Handle<decisiontree::Node> thisNodePtr = tree[k];
                    if(thisNodePtr->nodeID == nodeID){
                        if(thisNodePtr->leftChild == -1){
                            thisNodePtr->leftChild = thisChild;
                        }else{
                            thisNodePtr->rightChild = thisChild;
                        }
                    }
                }
            }
            forest.push_back(tree);
        }
        std::cout << "numbers of trees in the forest: " << forest.size() << std::endl;
        std::cout << "numbers of nodes in each tree: " << std::endl;
        for(int i = 0; i < forest.size(); i++){
            std::cout << "numbers of nodes in tree " << i << " is " << forest[i].size() << std::endl;
        }

        try{
            pdb::Handle<decisiontree::RandomForest> wholeRandomForest = pdb::makeObject<decisiontree::RandomForest>(forest);
            storeMe->push_back(wholeRandomForest);

            if (!pdbClient.sendData<decisiontree::RandomForest>(pair<string, string>(setName, dbName), storeMe, errMsg)) {
                std::cout << "Failed to send data to dispatcher server" << std::endl;
                return -1;
            }
        } catch (pdb::NotEnoughSpace& e) {
            if (!pdbClient.sendData<decisiontree::RandomForest>(pair<string, string>(setName, dbName), storeMe, errMsg)) {
                std::cout << "Not able to store data: " + errMsg;
                return -1;
            }
        }
        std::cout << "Successfully stored the tree data in Random Forest" << std::endl;
        pdbClient.flushData(errMsg);
    } else {
        std::cout << "Not create a set and not load new data to the Random Forest set" << std::endl;
    }

    makeObjectAllocatorBlock(1024 * 1024 * 1024, true);
    pdb::PDBLoggerPtr clientLoggerDT = make_shared<pdb::PDBLogger>("DecisionTreeClientLog");
    pdb::PDBClient pdbClientDT(8108, masterIp, clientLoggerDT, false, true);
    pdb::CatalogClient catalogClientDT(8108, masterIp, clientLoggerDT);

    if(createSetOrNot == true){
        ff::createDatabase(pdbClientDT, "decisiontree");
        ff::loadLibrary(pdbClientDT, "libraries/libFFMatrixMeta.so");
        ff::loadLibrary(pdbClientDT, "libraries/libFFMatrixData.so");
        ff::loadLibrary(pdbClientDT, "libraries/libFFMatrixBlock.so");
        ff::loadLibrary(pdbClientDT, "libraries/libFFMatrixBlockScanner.so");
        ff::loadLibrary(pdbClientDT, "libraries/libFFMatrixWriter.so");
        ff::loadLibrary(pdbClientDT, "libraries/libFFMatrixPartitioner.so");
        ff::createSet(pdbClientDT, "decisiontree", "inputs", "inputs", 64);
        ff::createSet(pdbClientDT, "decisiontree", "labels", "labels", 64);
        ff::loadLibrary(pdbClientDT, "libraries/libRFJoin.so");
        ff::loadLibrary(pdbClientDT,"libraries/libTreeNode.so");
        ff::loadLibrary(pdbClientDT,"libraries/libTree.so");
        ff::loadLibrary(pdbClientDT,"libraries/libRandomForest.so");
        ff::loadMatrix(pdbClientDT, "decisiontree", "inputs", rowNum, colNum, block_x,block_y, false, false, errMsg);
    } else{
        std::cout << "Not create a set and not load new data to the input set" << std::endl;
    }

    auto begin = std::chrono::high_resolution_clock::now();
    
    pdb::Handle<pdb::Computation> input1 = pdb::makeObject<ScanUserSet<decisiontree::RandomForest>>(dbName, setName);
    pdb::Handle<pdb::Computation> inputMatrix = pdb::makeObject<FFMatrixBlockScanner>("decisiontree", "inputs");

    pdb::Handle<pdb::Computation> join = pdb::makeObject<decisiontree::RFJoin>();
    join->setInput(0, input1);
    join->setInput(1, inputMatrix);

    pdb::Handle<pdb::Computation> labelWriter = pdb::makeObject<FFMatrixWriter>("decisiontree", "labels");
    labelWriter->setInput(join);

    bool materializeHash = true;
    //std::cout << "To run the Computation" << std::endl;
    auto exe_begin = std::chrono::high_resolution_clock::now();

    if (!pdbClientDT.executeComputations(errMsg, "decisiontree", materializeHash, labelWriter)) {
    	cout << "Computation failed. Message was: " << errMsg << "\n";
    	exit(1);
    }

    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "****Random Forest Join Execution Time Duration: ****"
              << std::chrono::duration_cast<std::chrono::duration<double>>(end - exe_begin).count()
              << " secs." << std::endl;
    
    std::cout << "****Random Forest Join End-End Time Duration: ****"
              << std::chrono::duration_cast<std::chrono::duration<double>>(end - begin).count()
              << " secs." << std::endl;

    //verify the results
    std::cout << "To print the status" << std::endl;
    ff::print_stats(pdbClientDT, "decisiontree", "labels");
    std::cout << "To print the results" << std::endl;
    ff::print(pdbClientDT, "decisiontree", "labels");
	return 0;
}

#endif