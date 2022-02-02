#ifndef TEST_MATERIALIZED_MODEL_CC
#define TEST_MATERIALIZED_MODEL_CC

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <cstring>

#include "PDBClient.h"
#include "StorageClient.h"
#include "TreeNode.h"
#include "FFMatrixBlockScanner.h"
#include "FFTransposeMult.h"
#include "FFAggMatrix.h"
#include "FFMatrixWriter.h"
#include "FFMatrixBlock.h"
#include "FFMatrixUtil.h"
#include "SpecializedBC.h"
#include "GenericDT.h"
#include "SimpleFF.h"

using namespace pdb;

int main(int argc, char *argv[]) {

	int rowNum = 569;
	int colNum = 31;
	int block_x = 569;
	int block_y = 31;

	string errMsg;

	makeObjectAllocatorBlock(1024 * 1024 * 1024, true);

	string masterIp = "localhost";
	pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("clientLog");
	pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
	pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);

	pdbClient.registerType("libraries/libTreeNode.so", errMsg);

	// create a new database
	string dbName = "mm_db";
    if (!pdbClient.createDatabase(dbName, errMsg)) {
        cout << "Not able to create database: " + errMsg;
        return -1;
    } else {
        std::cout << "Created materialized model database" << std::endl;
    }

    // create a new set in the database
    string setName = "mm_set";
    if (!pdbClient.createSet<decisiontree::Node>(dbName, setName, errMsg)) {
        cout << "Not able to create set: " + errMsg;
        return -1;
    } else {
        std::cout << "Created materialized model set" << std::endl;
    }

    pdb::Handle<pdb::Vector<pdb::Handle<decisiontree::Node>>> storeMe = pdb::makeObject<pdb::Vector<pdb::Handle<decisiontree::Node>>>();
    try{
        pdb::Handle<decisiontree::Node> myData;
        myData = pdb::makeObject<decisiontree::Node>(0,7,false,1,2,0.052);
        storeMe->push_back(myData);
        myData = pdb::makeObject<decisiontree::Node>(1,20,false,3,4,16.54);
        storeMe->push_back(myData);
        myData = pdb::makeObject<decisiontree::Node>(2,26,false,5,6,0.225);
        storeMe->push_back(myData);
        myData = pdb::makeObject<decisiontree::Node>(3,13,false,7,8,37.61);
        storeMe->push_back(myData);
        myData = pdb::makeObject<decisiontree::Node>(4,21,false,9,10,20.22);
        storeMe->push_back(myData);
        myData = pdb::makeObject<decisiontree::Node>(5,-1,true,-1,-1,2.0);
        storeMe->push_back(myData);
        myData = pdb::makeObject<decisiontree::Node>(6,23,false,11,12,710.2);
        storeMe->push_back(myData);
        myData = pdb::makeObject<decisiontree::Node>(7,21,false,13,14,33.27);
        storeMe->push_back(myData);
        myData = pdb::makeObject<decisiontree::Node>(8,4,false,15,16,0.091);
        storeMe->push_back(myData);
        myData = pdb::makeObject<decisiontree::Node>(9,-1,true,-1,-1,2.0);
        storeMe->push_back(myData);
        myData = pdb::makeObject<decisiontree::Node>(10,17,false,17,18,0.011);
        storeMe->push_back(myData);
        myData = pdb::makeObject<decisiontree::Node>(11,21,false,19,20,25.95);
        storeMe->push_back(myData);
        myData = pdb::makeObject<decisiontree::Node>(12,1,false,21,22,14.12);
        storeMe->push_back(myData);
        myData = pdb::makeObject<decisiontree::Node>(13,-1,true,-1,-1,2.0);
        storeMe->push_back(myData);
        myData = pdb::makeObject<decisiontree::Node>(14,21,false,23,24,34.14);
        storeMe->push_back(myData);
        myData = pdb::makeObject<decisiontree::Node>(15,-1,true,-1,-1,2.0);
        storeMe->push_back(myData);
        myData = pdb::makeObject<decisiontree::Node>(16,17,false,25,26,0.012);
        storeMe->push_back(myData);
        myData = pdb::makeObject<decisiontree::Node>(17,-1,true,-1,-1,1.0);
        storeMe->push_back(myData);
        myData = pdb::makeObject<decisiontree::Node>(18,-1,true,-1,-1,2.0);
        storeMe->push_back(myData);
        myData = pdb::makeObject<decisiontree::Node>(19,-1,true,-1,-1,2.0);
        storeMe->push_back(myData);
        myData = pdb::makeObject<decisiontree::Node>(20,9,false,27,28,0.065);
        storeMe->push_back(myData);
        myData = pdb::makeObject<decisiontree::Node>(21,25,false,29,30,0.361);
        storeMe->push_back(myData);
        myData = pdb::makeObject<decisiontree::Node>(22,-1,true,-1,-1,1.0);
        storeMe->push_back(myData);
        myData = pdb::makeObject<decisiontree::Node>(23,-1,true,-1,-1,1.0);
        storeMe->push_back(myData);
        myData = pdb::makeObject<decisiontree::Node>(24,-1,true,-1,-1,2.0);
        storeMe->push_back(myData);
        myData = pdb::makeObject<decisiontree::Node>(25,-1,true,-1,-1,2.0);
        storeMe->push_back(myData);
        myData = pdb::makeObject<decisiontree::Node>(26,-1,true,-1,-1,1.0);
        storeMe->push_back(myData);
        myData = pdb::makeObject<decisiontree::Node>(27,-1,true,-1,-1,2.0);
        storeMe->push_back(myData);
        myData = pdb::makeObject<decisiontree::Node>(28,-1,true,-1,-1,1.0);
        storeMe->push_back(myData);
        myData = pdb::makeObject<decisiontree::Node>(29,-1,true,-1,-1,1.0);
        storeMe->push_back(myData);
        myData = pdb::makeObject<decisiontree::Node>(30,-1,2.0,true,-1,-1);
        storeMe->push_back(myData);
        if (!pdbClient.sendData<decisiontree::Node>(pair<string, string>(setName, dbName), storeMe, errMsg)) {
            std::cout << "Failed to send data to dispatcher server" << std::endl;
            return -1;
        }
    } catch (pdb::NotEnoughSpace& e) {
    	if (!pdbClient.sendData<decisiontree::Node>(pair<string, string>(setName, dbName), storeMe, errMsg)) {
    		std::cout << "Not able to store data: " + errMsg;
    		return -1;
    	}
    }
    std::cout << "Successfully stored the data" << std::endl;
    pdbClient.flushData(errMsg);

    bool results = pdbClient.MM_getSet(dbName, setName, errMsg);
    std::cout << "Results from Server allocating memory to store pointer: " << results << std::endl;

    makeObjectAllocatorBlock(1024 * 1024 * 1024, true);
    pdb::PDBLoggerPtr clientLoggerDT = make_shared<pdb::PDBLogger>("DecisionTreeClientLog");
    pdb::PDBClient pdbClientDT(8108, masterIp, clientLoggerDT, false, true);
    pdb::CatalogClient catalogClientDT(8108, masterIp, clientLoggerDT);

    ff::createDatabase(pdbClientDT, "decisiontreeBC");

    ff::loadLibrary(pdbClientDT, "libraries/libFFMatrixMeta.so");
    ff::loadLibrary(pdbClientDT, "libraries/libFFMatrixData.so");
    ff::loadLibrary(pdbClientDT, "libraries/libFFMatrixBlock.so");
    ff::loadLibrary(pdbClientDT, "libraries/libFFMatrixBlockScanner.so");
    ff::loadLibrary(pdbClientDT, "libraries/libFFMatrixWriter.so");
    ff::loadLibrary(pdbClientDT, "libraries/libFFMatrixPartitioner.so");

    ff::createSet(pdbClientDT, "decisiontreeBC", "inputs", "inputs", 64);
    ff::createSet(pdbClientDT, "decisiontreeBC", "labels", "labels", 64);

    std::cout << "To load GenericDT shared library" << std::endl;
    ff::loadLibrary(pdbClientDT, "libraries/libGenericDT.so");
    ff::loadLibrary(pdbClientDT, "libraries/libSpecializedBC.so");

    auto begin = std::chrono::high_resolution_clock::now();

    std::cout << "To load matrix for decision tree inputs" << std::endl;
    ff::loadMatrix(pdbClientDT, "decisiontreeBC", "inputs", rowNum, colNum, block_x,
                   block_y, false, false, errMsg);

    std::cout << "To print the inputs" << std::endl;
    ff::print(pdbClientDT, "decisiontreeBC", "inputs");

    pdb::Handle<pdb::Computation> inputMatrix = pdb::makeObject<FFMatrixBlockScanner>("decisiontreeBC", "inputs");

    std::cout << "To make object of decision tree shared libraries" << std::endl;

    pdb::String fileName = dbName+setName;

    pdb::Handle<pdb::Computation> genericDT = pdb::makeObject<GenericDT>(fileName);
    genericDT->setInput(inputMatrix);

    std::cout << "To set the Computation" << std::endl;
    pdb::Handle<pdb::Computation> labelWriter = nullptr;
    labelWriter = pdb::makeObject<FFMatrixWriter>("decisiontreeBC", "labels");
    labelWriter->setInput(genericDT);

    bool materializeHash = false;
    std::cout << "To run the Computation" << std::endl;
    auto exe_begin = std::chrono::high_resolution_clock::now();

    if (!pdbClientDT.executeComputations(errMsg, "decisiontreeBC", materializeHash, labelWriter)) {
    	cout << "Computation failed. Message was: " << errMsg << "\n";
    	exit(1);
    }

    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "****Decision Tree End-to-End Time Duration: ****"
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;

    std::cout << "****Decision Tree Execution Time Duration: ****"
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - exe_begin).count()
              << " secs." << std::endl;
              
    //verify the results
    std::cout << "To print the status" << std::endl;
    ff::print_stats(pdbClientDT, "decisiontreeBC", "labels");
    std::cout << "To print the results" << std::endl;
    ff::print(pdbClientDT, "decisiontreeBC", "labels");
    return 0;
}

#endif